#include "cars.h"
#include "hero.h"
#include "particles.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>
#include <numeric>

inline std::vector<sf::Color> COLORS;

static constexpr int WIDTH = 2000;
static constexpr int HEIGHT = 1000;

int GameLoop(Particles& particles) {
    COLORS = {sf::Color::Cyan, sf::Color::Red, sf::Color::Blue, sf::Color::Green};

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window({WIDTH, HEIGHT}, "Game", sf::Style::Default, settings);

    sf::Clock clock;
    sf::Int64 lag = 0;
    const int usPerUpdate = 30000;

    GravityForce gravity = {
        1000,
        {WIDTH / 2, HEIGHT / 2},
    };

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
                {0, 0},
//                sf::Vector2f(Rand(-0.3f, 0.3f), Rand(0.3f, 0.3f)),
                sf::Vector2f(Rand(-2, 2), Rand(0, 10)),
                sf::Vector2f(Rand(10, 20), Rand(10, 20))
            );
        }

        window.clear(PEACH_PUFF);
        auto elapsed = clock.restart();
        lag += elapsed.asMicroseconds();

        if (lag >= usPerUpdate) {
//            hero.Update();
//            std::cerr << "tick: " << tick << std::endl;
            gravity.Apply(particles.physics);
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
        gravity.Render(window);
        window.display();
    }

    return 0;
}

void Main() {
    if (!PURISA_FONT.loadFromFile("/usr/share/fonts/truetype/tlwg/Purisa-Bold.ttf")) {
        throw std::runtime_error("No font!");
    }
    Particles particles;
//    particles.Add({0, HEIGHT - 100}, {0, 0}, {0, 0}, {WIDTH, 50}, 1000000);
    particles.Add(500, {WIDTH / 2 - 500, HEIGHT / 2 - 400, 1000, 1000});
    GameLoop(particles);
}

void TestBadCase() {
    Particles particles;
    GameLoop(particles);
}

const inline sf::Color GREY = {0x80, 0x80, 0x80};

struct WaterBottle {
    WaterBottle() {
        borders_.emplace_back(WindXy(50, 500));
        borders_.back().setPosition(WindXy(200, 200));
        borders_.emplace_back(WindXy(1000, 50));
        borders_.back().setPosition(WindXy(200, 700));
        borders_.emplace_back(WindXy(50, 500));
        borders_.back().setPosition(WindXy(1150, 200));
        for (auto& border : borders_) {
            border.setFillColor(GREY);
        }
        const int width = 1150 - 200 - 50;
        heights_.resize(width / dx_);
        for (int i = 0; i < width / dx_; ++i) {
            heights_[i] = i * 5;
        }
    }

    void Render(sf::RenderWindow& window) const {
        for (const auto& border : borders_) {
            window.draw(border);
        }
        for (int i = 0; i < heights_.size(); ++i) {
            const auto x = 250 + i * dx_;
            const auto y = heights_[i];
            auto rect = sf::RectangleShape(WindXy(dx_, y));
            rect.setPosition(WindXy(x, 700 - y));
            rect.setFillColor(sf::Color::Cyan);
            window.draw(rect);
//            window.dra
        }
    }

    std::vector<sf::RectangleShape> borders_;
    const int dx_ = 10;
    std::vector<double> heights_;
};

void BottleOfWater() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window({WIDTH, HEIGHT}, "Game", sf::Style::Default, settings);
    WaterBottle bottleOfWater;
    while (window.isOpen()) {
        sf::Event event;
        if (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::Closed) {
                std::exit(0);
            }
        }
        window.clear(PEACH_PUFF);
        bottleOfWater.Render(window);
        window.display();
    }
}

void MainCars() {
    std::cerr << sizeof(sf::FloatRect) << std::endl;
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window({WIDTH, HEIGHT}, "Game", sf::Style::Default, settings);
    Cars cars;
    while (window.isOpen()) {
        sf::Event event;
        if (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::Closed) {
                std::exit(0);
            }
        }
        cars.Update(window);
        window.clear(PEACH_PUFF);
        window.display();
    }
}

int main() {
//    Main();
//    BottleOfWater();
    MainCars();
}
