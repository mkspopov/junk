#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <vector>

struct Cars {
    std::vector<sf::FloatRect> pos_;
    std::vector<sf::Vector2f> vel_;
    std::vector<int> from_;
    std::vector<int> to_;

    void Render(sf::RenderWindow& window) const {
        for (auto pos : pos_) {
            auto rect = sf::RectangleShape(WindXy(pos.width, pos.height));
            rect.setPosition(pos.left, pos.top);
            rect.setFillColor(GREY);
            window.draw(rect);
        }
    }

    void Update(sf::RenderWindow& window) {
        static CollisionDetector<Cars> collisionDetector;
        float timeLeft = 1;
        collisionDetector.Detect(*this, timeLeft);
    }
};
