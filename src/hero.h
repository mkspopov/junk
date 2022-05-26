#pragma once

#include "particles.h"

#include <SFML/Window/Event.hpp>

namespace hero {

struct Hero {
    Hero() {
        physics.rects.emplace_back(sf::Vector2f(50, 50), WindXy(0, 0));

        keyPressed_.fill(false);

        physics.velocities.emplace_back();
        physics.masses.push_back(100);
    }

    void HandleInput(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
            case sf::Keyboard::W:
                keyPressed_[sf::Keyboard::W] = true;
                break;
            case sf::Keyboard::S:
                keyPressed_[sf::Keyboard::S] = true;
                break;
            case sf::Keyboard::A:
                keyPressed_[sf::Keyboard::A] = true;
                break;
            case sf::Keyboard::D:
                keyPressed_[sf::Keyboard::D] = true;
                break;
            default:
                break;
            }
        } else if (event.type == sf::Event::KeyReleased) {
            switch (event.key.code) {
            case sf::Keyboard::W:
                keyPressed_[sf::Keyboard::W] = false;
                break;
            case sf::Keyboard::S:
                keyPressed_[sf::Keyboard::S] = false;
                break;
            case sf::Keyboard::A:
                keyPressed_[sf::Keyboard::A] = false;
                break;
            case sf::Keyboard::D:
                keyPressed_[sf::Keyboard::D] = false;
                break;
            default:
                break;
            }
        }

        auto& velocity = physics.velocities.back();
        if (keyPressed_[sf::Keyboard::W]) {
            if (keyPressed_[sf::Keyboard::A]) {
                velocity.x = -SPEED / std::numbers::sqrt2;
                velocity.y = -SPEED / std::numbers::sqrt2;
            } else if (keyPressed_[sf::Keyboard::D]) {
                velocity.x = SPEED / std::numbers::sqrt2;
                velocity.y = -SPEED / std::numbers::sqrt2;
            } else {
                velocity.y = -SPEED;
                velocity.x = 0;
            }
        } else if (keyPressed_[sf::Keyboard::S]) {
            if (keyPressed_[sf::Keyboard::A]) {
                velocity.x = -SPEED / std::numbers::sqrt2;
                velocity.y = SPEED / std::numbers::sqrt2;
            } else if (keyPressed_[sf::Keyboard::D]) {
                velocity.x = SPEED / std::numbers::sqrt2;
                velocity.y = SPEED / std::numbers::sqrt2;
            } else {
                velocity.y = SPEED;
                velocity.x = 0;
            }
        } else if (keyPressed_[sf::Keyboard::A]) {
            velocity.x = -SPEED;
            velocity.y = 0;
        } else if (keyPressed_[sf::Keyboard::D]) {
            velocity.x = SPEED;
            velocity.y = 0;
        } else {
            velocity.x = 0;
            velocity.y = 0;
        }
    }

    void Render(sf::RenderWindow& window, float dt) {
        auto shape = sf::RectangleShape({physics.rects.back().width, physics.rects.back().height});
        shape.setPosition(physics.rects.back().left, physics.rects.back().top);
        shape.setFillColor(sf::Color::Red);
        shape.move(physics.velocities.back() * dt);
        window.draw(shape);
    }

    void Update() {
         Move(physics.rects.back(), physics.velocities.back());
    }

    Physics physics;
    std::array<bool, sf::Keyboard::KeyCount> keyPressed_;
    static constexpr float SPEED = 10;
};

}  // namespace hero
