#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Font.hpp>

#include <memory>
#include <mutex>
#include <random>

inline std::size_t tick = 0;

inline sf::Font PURISA_FONT;

namespace std {
template <class T>
ostream& operator<<(ostream& os, const sf::Vector2<T>& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
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
