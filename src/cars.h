#pragma once

#include "particles.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <vector>

struct Cars {
    void Add(WindXy at, sf::Vector2f velocity, WindXy finish) {
        sf::FloatRect rect = {at, {30, 30}};
        for (const auto& other : physics.rects) {
            if (rect.intersects(other)) {
                return;
            }
        }
        physics.PushBack(rect, velocity, WindXy(0, 0), 1);
        to.push_back(finish);
    }

    static void CollisionsCallback(
        Physics& physics,
        float dt,
        const std::vector<std::size_t>& hitIndices,
        std::vector<Locked<FirstHit>>& firstHits)
    {
        if (dt > 0) {
            // Move all before the first hit
            for (std::size_t i = 0; i < physics.Size(); ++i) {
                Move(physics.rects[i], physics.velocities[i] * dt);
            }
        }

        for (std::size_t i : hitIndices) {
            // Cancel above Move
            Move(physics.rects[i], -physics.velocities[i] * dt);

            const auto j = firstHits[i]->otherIndex;
            physics.SetVelocity(i, {0, 0});
            physics.SetVelocity(j, {0, 0});
        }
    }

    void Erase(std::size_t index) {
        physics.Erase(index);
        to.erase(to.begin() + index);
    }

    void Render(sf::RenderWindow& window, float part) const {
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            auto rect = physics.rects[i];
            auto shape = sf::RectangleShape(WindXy(rect.width, rect.height));
            shape.setPosition(rect.left, rect.top);
            shape.move(physics.velocities[i] * part);
            shape.setFillColor(GREY);
            window.draw(shape);
        }
    }

    void Update(sf::RenderWindow& window) {
//        for (std::size_t i = 0; i < physics.Size(); ++i) {
//            physics.SetVelocity(i, SPEED * Normed(to[i] - GetPosition(physics.rects[i])));
////            physics.SetVelocity(i, physics.velocities[i] + physics.accelerations[i]);
//        }

        static CollisionDetector detector;
        detector.Clear();

        float timeLeft = 1;
        while (timeLeft > 0 && detector.Detect(physics, timeLeft, CollisionsCallback)) {
        }

        if (timeLeft > 0) {
            for (std::size_t i = 0; i < physics.Size(); ++i) {
                Move(physics.rects[i], physics.velocities[i] * timeLeft);
            }
        }

        std::vector<std::size_t> dead;
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            const auto& rect = physics.rects[i];
            const float windX = window.getSize().x;
            const float windY = window.getSize().y;
            if (rect.left > windX ||
                rect.top > windY ||
                rect.left + rect.width < -windX ||
                rect.top + rect.height < -windY)
            {
                dead.push_back(i);
            }
        }
        for (auto i = dead.rbegin(); i != dead.rend(); ++i) {
            physics.Erase(*i);
            to.erase(to.begin() + *i);
        }
    }

    Physics physics;
    std::vector<WindXy> to;
    static constexpr float SPEED = 10;
};
