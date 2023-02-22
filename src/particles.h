#pragma once

#include "physics.h"
#include "utils.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>
#include <iostream>
#include <ostream>
#include <thread>
#include <unordered_set>
#include <vector>
#include <bitset>

namespace particles {

struct FirstHit {
    bool CanHit(std::size_t index) const {
        return !cannotHitSet.contains(index);
    }

    void AddHit(float start, char coordinate, std::size_t otherIndex) {
        if (start < time) {
            time = start;
            coords.clear();
            coords.push_back(coordinate);
            otherIndices.clear();
            otherIndices.push_back(otherIndex);
        } else if (start == time) {
            coords.push_back(coordinate);
            otherIndices.push_back(otherIndex);
        }
    }

    std::vector<char> coords;
    // otherIndices are greater than this index
    std::vector<std::size_t> otherIndices;
    std::unordered_set<std::size_t> cannotHitSet;
    float time = 1;
};

struct GravityForce {
    void Apply(Physics& physics) const {
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            if (!physics.properties[i].test(Physics::Properties::Gravity)) {
                continue;
            }
            const auto direction = position - Center(physics.shapes[i]);
            const auto distance = std::abs(direction);
            physics.velocities[i] += GRAVITY_CONST * mass / std::pow(distance, 3.f) * direction;
        }
    }

    void Render(sf::RenderTarget& window) {
        sf::CircleShape circle(3);
        circle.setPosition(position);
        circle.setFillColor(sf::Color::Magenta);
        window.draw(circle);
    }

    float mass;
    WindXy position;
};

template <class TShape>
struct Particles {
    void Add(int num, sf::FloatRect where);

    bool Add(WindXy at, sf::Vector2f acceleration, sf::Vector2f velocity, sf::Vector2f size,
        float density = 1);

    bool Add(float atx, float aty, float ax, float ay, float vx, float vy, float sx, float sy,
        float density = 1);

    static void CollisionsCallback(
        Physics<TShape>& physics,
        float dt,
        const std::vector<std::size_t>& hitIndices,
        std::vector<Locked<FirstHit>>& firstHits);

    void HandleInput(const sf::Event& event);

    void Render(sf::RenderTarget& window, float part);

    void Update(sf::RenderTarget& window, const std::vector<Physics*>& others);

    Physics<TShape> physics;

    std::vector<std::pair<std::unique_ptr<sf::Shape>, int>> toRender;
};

template <class TShape>
class CollisionDetector {
    using TCallback = std::function<void(
        Physics<TShape>& physics,
        float dt,
        const std::vector<std::size_t>& hitIndices,
        std::vector<Locked<FirstHit>>& firstHits)>;
public:
    void Clear();

    bool Detect(Physics<TShape>& physics, float& timeLeft, TCallback callback);

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

}  // namespace particles
