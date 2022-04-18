#include "hero.h"
#include "particles.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>
#include <numeric>

inline std::vector<sf::Color> COLORS;

int GameLoop(Particles& particles) {
    COLORS = {sf::Color::Cyan, sf::Color::Red, sf::Color::Blue, sf::Color::Green};

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window({WIDTH, HEIGHT}, "Game", sf::Style::Default, settings);

    sf::Clock clock;
    sf::Int64 lag = 0;
    const int usPerUpdate = 30000;
    std::size_t tick = 0;

//    hero::Hero hero;

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
//            hero.HandleInput(event);
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
//            hero.Update();
//            std::cerr << "tick: " << tick << std::endl;
            particles.Update(window, {});
//            particles.Update(window, {&hero.physics});
            lag -= usPerUpdate;
            ++tick;
        }

        const bool smooth = true;
        if (smooth) {
//            hero.Render(window, static_cast<float>(lag) / usPerUpdate);
            particles.Render(window, static_cast<float>(lag) / usPerUpdate);
        } else {
//            hero.Render(window, 0);
            particles.Render(window, 0);
        }
        window.display();
    }

    return 0;
}

void Main() {
    Particles particles;
    particles.Add(500, {WIDTH / 2 - 500, HEIGHT / 2 - 400, 1000, 1000});
    GameLoop(particles);
}

void TestBadCase() {
    Particles particles;
    particles.Add(535.78, 327.676, -2, -3, 20, 28);
    particles.Add(515.377, 308.819, 2, 1, 26, 28);
    GameLoop(particles);
}

int main() {
    Main();
//    TestBadCase();
}
