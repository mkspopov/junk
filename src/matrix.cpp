#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

#include <immintrin.h>
#include <avx2intrin.h>
#include <random>

enum class MultType {
    Fast,
    Slow,
};

template <MultType multType = MultType::Fast>
class Matrix {
public:
    Matrix(std::initializer_list<std::initializer_list<float>> list)
        : data_(list.begin(), list.end())
    {}

    Matrix(std::size_t rows, std::size_t cols, float value = 0)
        : data_(rows, std::vector<float>(cols, value))
    {}

    Matrix operator*(const Matrix& other) const {
        assert(!data_.empty() && !data_[0].empty());
        assert(!other.data_.empty() && !other.data_[0].empty());
        assert(data_[0].size() == other.data_.size());
        Matrix out(data_.size(), other.data_[0].size());

        if constexpr (multType == MultType::Slow) {
            for (int i = 0; i < out.data_.size(); ++i) {
                for (int j = 0; j < out.data_[0].size(); ++j) {
                    for (int k = 0; k < data_[0].size(); ++k) {
                        out.data_[i][j] += data_[i][k] * other.data_[k][j];
                    }
                }
            }
        } else {
            for (int i = 0; i < out.data_.size(); ++i) {
                for (int j = 0; j < out.data_[0].size(); j += 2) {
                    for (int k = 0; k < data_[0].size(); k += 4) {
                        __v8si index = {k, k+1, k+2, k+3, k, k+1, k+2, k+3};
                        auto lhs = _mm256_i32gather_ps(data_[i].data(), index, 4);
                        __m256 rhs;
                        for (int n = 0; n < 4; ++n) {
                            rhs[n] = other.data_[k + n][j];
                            rhs[n + 4] = other.data_[k + n][j + 1];
                        }
                        auto res = _mm256_dp_ps(lhs, rhs, 0b11110001);
                        out.data_[i][j] += res[0];
                        out.data_[i][j + 1] += res[4];
                    }
                }
            }
        }

        return out;
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix& m) {
        assert(!m.data_.empty() && !m.data_[0].empty());
        const auto cols = m.data_[0].size();
        for (const auto& row : m.data_) {
            for (std::size_t j = 0; j < cols - 1; ++j) {
                os << row[j] << ' ';
            }
            os << row[cols - 1] << '\n';
        }
        return os;
    }

    template <MultType TR>
    bool operator==(const Matrix<TR>& rhs) const {
        if (data_.size() != rhs.data_.size() || data_[0].size() != rhs.data_[0].size()) {
            return false;
        }
        for (int i = 0; i < data_.size(); ++i) {
            for (int j = 0; j < data_[0].size(); ++j) {
                if (std::abs(data_[i][j] - rhs.data_[i][j]) > 1e-4) {
                    return false;
                }
            }
        }
        return true;
    }

    float& operator()(std::size_t i, std::size_t j) {
        return data_[i][j];
    }

private:
    friend class Matrix<MultType::Slow>;
    friend class Matrix<MultType::Fast>;

    std::vector<std::vector<float>> data_;
};

int main() {
    Matrix<MultType::Fast> a = {
        {1, 2, 3, 4, 1, 1, 1, 1},
        {5, 6, 7, 8, 1, 1, 1, 1},
        {7, 6, 5, 4, 1, 1, 1, 1},
        {3, 2, 1, 0, 1, 1, 1, 1},
        {1, 2, 3, 4, 1, 1, 1, 1},
        {5, 6, 7, 8, 1, 1, 1, 1},
        {7, 6, 5, 4, 1, 1, 1, 1},
        {3, 2, 1, 0, 1, 1, 1, 1},
    };
    Matrix<MultType::Slow> b = {
        {1, 2, 3, 4, 1, 1, 1, 1},
        {5, 6, 7, 8, 1, 1, 1, 1},
        {7, 6, 5, 4, 1, 1, 1, 1},
        {3, 2, 1, 0, 1, 1, 1, 1},
        {1, 2, 3, 4, 1, 1, 1, 1},
        {5, 6, 7, 8, 1, 1, 1, 1},
        {7, 6, 5, 4, 1, 1, 1, 1},
        {3, 2, 1, 0, 1, 1, 1, 1},
    };

    std::cout << a * a << std::endl;
    std::cout << b * b << std::endl;
    assert(a * a == b * b);

    std::size_t size = 10000;
    Matrix<MultType::Slow> big(size, size);
    Matrix<MultType::Fast> bigFast(size, size);
    std::mt19937 gen(7);
    std::uniform_real_distribution<float> dis(-1, 1);
    for (std::size_t i = 0; i < size; ++i) {
        for (std::size_t j = 0; j < size; ++j) {
            big(i, j) = dis(gen);
        }
    }
    for (std::size_t i = 0; i < size; ++i) {
        for (std::size_t j = 0; j < size; ++j) {
            bigFast(i, j) = big(i, j);
        }
    }

    auto start = std::chrono::steady_clock::now();
    auto slowRes = big * big;
    (void) slowRes;
    auto middle = std::chrono::steady_clock::now();
    auto fastRes = bigFast * bigFast;
    (void) fastRes;
    auto finish = std::chrono::steady_clock::now();

    std::cout << "Slow: " << (middle - start).count() << std::endl;
    std::cout << "Fast: " << (finish - middle).count() << std::endl;
    if (slowRes != fastRes) {
        throw std::runtime_error("Not equal");
    }
}
