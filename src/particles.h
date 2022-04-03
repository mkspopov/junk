#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>
#include <random>
#include <iostream>
#include <ostream>
#include <thread>

using WindXy = sf::Vector2f;

inline std::mt19937 gen;

inline const auto PEACH_PUFF = sf::Color(0xFFDAB9FF);
inline const auto LIGHT_GREY = sf::Color(0x7f7f7faa);
inline const auto DARK_GREY = sf::Color(0x6B6E76FF);
inline const auto SCARLET = sf::Color(0xfc2847ff);

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

struct Physics {
    void PushBack(sf::RectangleShape shape, sf::Vector2f velocity, float mass) {
        shapes.push_back(shape);
        velocities.push_back(velocity);
        masses.push_back(mass);
    }

    void PopBack() {
        shapes.pop_back();
        velocities.pop_back();
        masses.pop_back();
    }

    std::vector<sf::RectangleShape> shapes;
    std::vector<sf::Vector2f> velocities;
    std::vector<float> masses;
};

struct Particles {
    void Add(int num, sf::FloatRect where);

    bool Add(WindXy at, sf::Vector2f velocity, sf::Vector2f size, float density = 1);

    void Render(sf::RenderWindow& window, float part);

    void Update(sf::RenderWindow& window, const std::vector<Physics*>& others);

    Physics physics;

    std::vector<std::pair<std::unique_ptr<sf::Shape>, int>> toRender;
};
