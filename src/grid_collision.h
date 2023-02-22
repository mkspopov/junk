#pragma once

#include "physics.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <optional>

namespace grid {

/*
 * GridCollision is helpful for collision detection
 * of high density particles of almost the same size.
 */
class GridCollision {
public:
    explicit GridCollision(Physics<sf::CircleShape>& particles, std::optional<float> avgParticleSize) : particles_(particles) {
        if (!avgParticleSize) {
        }
    }

private:
    Physics<sf::CircleShape>& particles_;
};

}  // namespace grid
