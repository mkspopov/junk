#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <complex>
#include <memory>
#include <mutex>
#include <random>

inline std::size_t tick = 0;

using WindXy = sf::Vector2f;

inline sf::Font PURISA_FONT;

static constexpr int WIDTH = 2000;
static constexpr int HEIGHT = 1000;

static constexpr int DPIXELS = 5;

inline const auto PEACH_PUFF = sf::Color(0xFFDAB9FF);
inline const auto LIGHT_GREY = sf::Color(0x7f7f7faa);
inline const auto DARK_GREY = sf::Color(0x6B6E76FF);
inline const auto SCARLET = sf::Color(0xfc2847ff);
const inline sf::Color GREY = {0x80, 0x80, 0x80};

namespace std {

template <class T>
ostream& operator<<(ostream& os, const sf::Vector2<T>& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

template <typename T>
inline std::complex<T> ToComplex(const sf::Vector2<T>& right) {
    return {right.x, right.y};
}

template <typename T>
inline float abs(const sf::Vector2<T>& v) {
    return abs(ToComplex(v));
}

}  // namespace std

template <typename T>
inline sf::Vector2<T> Normed(const sf::Vector2<T>& right) {
    return right / std::abs(right);
}

inline float Dot(const sf::Vector2f& left, const sf::Vector2f& right) {
    return left.x * right.x + left.y * right.y;
}

inline float Cos(const sf::Vector2f& left, const sf::Vector2f& right) {
    return Dot(left, right) / (std::abs(left) * std::abs(right));
}

inline WindXy Center(const sf::FloatRect& rect) {
    return {rect.left + rect.width / 2, rect.top + rect.height / 2};
}

inline void Move(sf::FloatRect& rect, sf::Vector2f velocity) {
    rect.left += velocity.x;
    rect.top += velocity.y;
}

inline WindXy GetPosition(const sf::FloatRect& rect) {
    return {rect.left, rect.top};
}

template <class T>
struct alignas(64) Locked {
public:
    template <class ...Args>
    Locked(Args&& ...args)
        : mutex_(std::make_unique<std::mutex>())
        , value_(std::forward<Args>(args)...)
    {}

    Locked(Locked&& rhs) noexcept
        : mutex_(std::move(rhs.mutex_))
        , value_(std::move(rhs.value_))
    {}

    Locked& operator=(Locked&& rhs) noexcept {
        mutex_ = std::move(rhs.mutex_);
        value_ = std::move(rhs.value_);
        return *this;
    }

    void lock() {
        mutex_->lock();
    }

    void unlock() {
        mutex_->unlock();
    }

    T* operator->() {
        return &value_;
    }

    T& operator*() {
        return value_;
    }

private:
    std::unique_ptr<std::mutex> mutex_;
    T value_;
};

inline std::mt19937 gen;
inline std::normal_distribution<float> NORMAL;

template <class T = float>
inline float Rand(T from = std::numeric_limits<T>::lowest(), T to = std::numeric_limits<T>::max()) {
    if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dis(from, to);
        return dis(gen);
    } else {
        std::uniform_int_distribution<T> dis(from, to);
        return dis(gen);
    }
}

inline std::map<std::string, double, std::less<>> gStats;

inline void DrawStats(sf::RenderWindow& window) {
    int height = 10;
    for (const auto& [name, value] : gStats) {
        sf::Text text(
            name + ": " + std::to_string(value),
            PURISA_FONT,
            20
        );
        text.setFillColor(sf::Color::Black);
        text.setPosition(WIDTH - 300, height);
        height += 40;
        window.draw(text);
    }
}

//template <class T>
//std::enable_if_t<std::is_arithmetic_v<T>, bool> AlmostEqual(T lhs, T rhs) {
//    return std::abs(rhs - lhs) <= 1e-6;
//}
