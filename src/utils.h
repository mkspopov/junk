#pragma once

#include <memory>
#include <mutex>
#include <random>

template <class T>
struct alignas(64) Locked {
public:
    template <class ...Args>
    Locked(Args&& ...args)
        : mutex_(std::make_unique<std::mutex>())
        , value_(std::forward<Args>(args)...)
    {}

    void lock() {
        mutex_->lock();
    }

    void unlock() {
        mutex_->unlock();
    }

    T* operator->() {
        return &value_;
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
