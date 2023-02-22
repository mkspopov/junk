#include "utils.h"

#include <SFML/System/Vector2.hpp>

#include <bitset>
#include <vector>

template <class TShape>
struct Physics {
    enum Properties {
        None = -1,
//        Collision = 1 << 3,
        Gravity,
//        Friction = 1 << 1,
//        Elasticity = 1 << 2,
        Move,
    };

    void Erase(std::size_t index) {
        shapes.erase(shapes.begin() + index);
        accelerations.erase(accelerations.begin() + index);
        velocities.erase(velocities.begin() + index);
        masses.erase(masses.begin() + index);
        properties.erase(properties.begin() + index);
    }

    void PushBack(
        TShape shape,
        sf::Vector2f velocity,
        sf::Vector2f acceleration,
        float mass)
    {
        shapes.push_back(std::move(shape));
        accelerations.push_back(acceleration);
        velocities.push_back(velocity);
        masses.push_back(mass);
        properties.emplace_back().set();
    }

    void PopBack() {
        shapes.pop_back();
        accelerations.pop_back();
        velocities.pop_back();
        masses.pop_back();
        properties.pop_back();
    }

    void SetVelocity(std::size_t index, sf::Vector2f velocity) {
        if (!properties[index].test(Move)) {
            return;
        }
        auto speed = std::abs(velocity);
        velocities[index] = velocity;
        if (speed > MAX_SPEED) {
            velocities[index] *= MAX_SPEED / speed;
        }
    }

    std::size_t Size() const {
        return shapes.size();
    }

    std::vector<TShape> shapes;
    std::vector<sf::Vector2f> accelerations;
    std::vector<sf::Vector2f> velocities;
    std::vector<float> masses;
    std::vector<std::bitset<8>> properties;
};
