#include <vector>
#include <iostream>

struct Addition;

template <class T, class U, class Op>
struct Node {
    std::size_t Rows() const {
        return lhs.Rows();
    }

    std::size_t Cols() const {
        return rhs.Cols();
    }

    T lhs;
    U rhs;
};

struct Matrix {
    Matrix() = default;

    template <class T, class U, class Op>
    Matrix(Node<T, U, Op> node) : m(node.Rows(), std::vector<double>(node.Cols())) {
        *this += node;
    }

    Matrix(const Matrix& rhs) = delete;
    Matrix& operator=(const Matrix& rhs) = delete;

    Matrix(Matrix&& rhs) = default;
    Matrix& operator=(Matrix&& rhs) = default;

    std::size_t Rows() const {
        return m.size();
    }

    std::size_t Cols() const {
        if (m.empty()) {
            return 0;
        }
        return m[0].size();
    }

    std::vector<std::vector<double>> m;
};

Matrix& operator+=(Matrix& lhs, const Matrix& rhs) {
    for (std::size_t i = 0; i < lhs.m.size(); ++i) {
        for (std::size_t j = 0; j < lhs.m[0].size(); ++j) {
            lhs.m[i][j] += rhs.m[i][j];
        }
    }
    return lhs;
}

template <class T, class U>
void MultiplyTo(const Matrix& lhs, const Matrix& rhs, Matrix& out) {
    out.m = std::vector<std::vector<double>>(lhs.Rows(), std::vector<double>(rhs.Cols()));
    for (int i = 0; i < out.Rows(); ++i) {
        for (int j = 0; j < out.Cols(); ++j) {
            for (int k = 0; k < lhs.Cols(); ++k) {
                out.m[i][j] += k;
            }
        }
    }
}

template <class T, class U, class Op>
Matrix& operator+=(Matrix& lhs, const Node<T, U, Op>& rhs) {
    if constexpr (std::is_same_v<Op, Addition>) {
        lhs += rhs.lhs;
        lhs += rhs.rhs;
    } else if constexpr (std::is_same_v<Op, Multiplication>) {
        // not necessary
//        MultiplyTo(lhs, )
    }
    return lhs;
}

template <class T, class U>
auto operator+(T&& lhs, U&& rhs) {
    return Node<T, U, Addition>{lhs, rhs};
}

int main() {
    Matrix a, b, c;
    Matrix d = a + b + c;
    std::cout << d.m.size() << std::endl;
}
