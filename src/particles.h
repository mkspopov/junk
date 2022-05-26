#pragma once

#include "utils.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <ostream>
#include <thread>
#include <unordered_set>
#include <vector>

inline float GRAVITY_CONST = 1;
inline float MAX_SPEED = 5;

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
    void Erase(std::size_t index) {
        rects.erase(rects.begin() + index);
        accelerations.erase(accelerations.begin() + index);
        velocities.erase(velocities.begin() + index);
        masses.erase(masses.begin() + index);
    }

    void PushBack(
        sf::FloatRect rect,
        sf::Vector2f velocity,
        sf::Vector2f acceleration,
        float mass)
    {
        rects.push_back(std::move(rect));
        accelerations.push_back(acceleration);
        velocities.push_back(velocity);
        masses.push_back(mass);
    }

    void PopBack() {
        rects.pop_back();
        accelerations.pop_back();
        velocities.pop_back();
        masses.pop_back();
    }

    void SetVelocity(std::size_t index, sf::Vector2f velocity) {
        auto speed = std::abs(velocity);
        velocities[index] = velocity;
        if (speed > MAX_SPEED) {
            velocities[index] *= MAX_SPEED / speed;
        }
    }

    std::size_t Size() const {
        return rects.size();
    }

    std::vector<sf::FloatRect> rects;
    std::vector<sf::Vector2f> accelerations;
    std::vector<sf::Vector2f> velocities;
    std::vector<float> masses;
};

struct GravityForce {
    void Apply(Physics& physics) const {
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            const auto direction = position - Center(physics.rects[i]);
            const auto distance = std::abs(direction);
            physics.velocities[i] += GRAVITY_CONST * mass / std::pow(distance, 3.f) * direction;
        }
    }

    void Render(sf::RenderWindow& window) {
        sf::CircleShape circle(3);
        circle.setPosition(position);
        circle.setFillColor(sf::Color::Magenta);
        window.draw(circle);
    }

    float mass;
    WindXy position;
};

struct Particles {
    void Add(int num, sf::FloatRect where);

    bool Add(WindXy at, sf::Vector2f acceleration, sf::Vector2f velocity, sf::Vector2f size, float density = 1);
    bool Add(float atx, float aty, float ax, float ay, float vx, float vy, float sx, float sy, float density = 1);

    static void CollisionsCallback(
        Physics& physics,
        float dt,
        const std::vector<std::size_t>& hitIndices,
        std::vector<Locked<FirstHit>>& firstHits);

    void Render(sf::RenderWindow& window, float part);

    void Update(sf::RenderWindow& window, const std::vector<Physics*>& others);

    Physics physics;

    std::vector<std::pair<std::unique_ptr<sf::Shape>, int>> toRender;
};

class CollisionDetector {
    using TCallback = std::function<void(
        Physics& physics,
        float dt,
        const std::vector<std::size_t>& hitIndices,
        std::vector<Locked<FirstHit>>& firstHits)>;
public:
    void Clear();

    bool Detect(Physics& physics, float& timeLeft, TCallback callback);

private:
    struct BoundingBox {
        sf::FloatRect rect;
        std::size_t index;
    };

    void CheckCollision(Physics& physics, std::size_t i, std::size_t j);
    void SimpleCheck(Physics& physics, const std::vector<BoundingBox>& boxes);
    void UpdateCollisions(Physics& physics, std::vector<BoundingBox>& boxes, bool sortByX);

    std::vector<BoundingBox> boxes_;
    std::vector<Locked<FirstHit>> firstHits_;
};
