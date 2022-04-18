#pragma once

#include "utils.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <ostream>
#include <thread>
#include <unordered_set>
#include <vector>

using WindXy = sf::Vector2f;

inline const auto PEACH_PUFF = sf::Color(0xFFDAB9FF);
inline const auto LIGHT_GREY = sf::Color(0x7f7f7faa);
inline const auto DARK_GREY = sf::Color(0x6B6E76FF);
inline const auto SCARLET = sf::Color(0xfc2847ff);

static constexpr int WIDTH = 1280;
static constexpr int HEIGHT = 720;

struct FirstHit {
    bool CanHit(std::size_t index) const {
        return !cannotHitSet.contains(index);
    }

    std::size_t otherIndex = -1;
    std::unordered_set<std::size_t> cannotHitSet;
    float time = 1;
    char coord;
};

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
    bool Add(float atx, float aty, float vx, float vy, float sx, float sy, float density = 1);

    void Render(sf::RenderWindow& window, float part);

    void Update(sf::RenderWindow& window, const std::vector<Physics*>& others);

    Physics physics;

    std::vector<std::pair<std::unique_ptr<sf::Shape>, int>> toRender;

private:
    std::vector<Locked<FirstHit>> firstHits_;
};
