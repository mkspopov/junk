#include "particles.h"

#include <SFML/Window/Event.hpp>

#include <iostream>
#include <numeric>

inline std::vector<sf::Color> COLORS;

static constexpr int WIDTH = 1280;
static constexpr int HEIGHT = 720;

struct Hero {
    Hero() {
        physics.shapes.emplace_back(sf::Vector2f(50, 50));
        physics.shapes.back().setFillColor(sf::Color::Red);
        physics.shapes.back().setPosition(WIDTH / 2, HEIGHT / 2);

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
        auto s = physics.shapes.back();
        s.move(physics.velocities.back() * dt);
        window.draw(s);
    }

    void Update() {
        physics.shapes.back().move(physics.velocities.back());
    }

    Physics physics;
    std::array<bool, sf::Keyboard::KeyCount> keyPressed_;
    static constexpr float SPEED = 10;
};

int main() {
    Particles particles;
    particles.Add(500, {WIDTH / 2 - 500, HEIGHT / 2 - 400, 1000, 1000});

    COLORS = {sf::Color::Cyan, sf::Color::Red, sf::Color::Blue};

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window({WIDTH, HEIGHT}, "Game", sf::Style::Default, settings);

    sf::Clock clock;
    sf::Int64 lag = 0;
    const int usPerUpdate = 30000;

    Hero hero;

    bool spammingBricks = false;
    while (window.isOpen()) {
        sf::Event event;
        if (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::Closed) {
                std::exit(0);
            } else if (event.type == sf::Event::EventType::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Button::Left) {
                    spammingBricks = true;
                }
            } else if (event.type == sf::Event::EventType::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Button::Left) {
                    spammingBricks = false;
                }
            }
            hero.HandleInput(event);
        }

        if (spammingBricks) {
            auto [x, y] = sf::Mouse::getPosition(window);
            particles.Add(
                WindXy(x, y),
                sf::Vector2f(Rand(-2, 2), Rand(0, 10)),
                sf::Vector2f(Rand(10, 20), Rand(10, 20))
            );
        }

        window.clear(PEACH_PUFF);
        auto elapsed = clock.restart();
        lag += elapsed.asMicroseconds();

        while (lag >= usPerUpdate) {
            hero.Update();
            particles.Update(window, {&hero.physics});
            lag -= usPerUpdate;
        }

        const bool smooth = true;
        if (smooth) {
            hero.Render(window, static_cast<float>(lag) / usPerUpdate);
            particles.Render(window, static_cast<float>(lag) / usPerUpdate);
        } else {
            hero.Render(window, 0);
            particles.Render(window, 0);
        }
        window.display();
    }

    return 0;
}
