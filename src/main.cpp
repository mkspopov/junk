#include "cars.h"
#include "graph.h"
#include "hero.h"
#include "particles.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include <cassert>
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
    sf::Clock clock;
    sf::Int64 lag = 0;
    const int usPerUpdate = 30000;

    while (window.isOpen()) {
        sf::Event event;
        if (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::Closed) {
                std::exit(0);
            }
        }
//        window.clear(PEACH_PUFF);
//        auto elapsed = clock.restart();
//        lag += elapsed.asMicroseconds();

//        if (lag >= usPerUpdate) {
//            game.Update(window);
//            lag -= usPerUpdate;
//        }

        bottleOfWater.Render(window);
        window.display();
    }
}

class Game {
public:
    Game() {
        sources = {
            {0, 100},
        };
        periods = {
            {50, 0},
        };
        targets = {
            {WIDTH, 900},
        };
    }

    void Update(sf::RenderWindow& window) {
        // choose direction
        for (std::size_t i = 0; i < cars.physics.Size(); ++i) {
            const auto vertex = GetVertex(Center(cars.physics.rects[i]));
            if (vertex < 0) {
                continue;
            }
            int minCost = 1;
            int bestDirection;
            Graph::Edge* edge = nullptr;
            for (int dirInd = 0; dirInd < Graph::DIRS.size(); ++dirInd) {
                auto cur = graph.GetEdge(vertex, dirInd);
                if (cur && minCost > cur->cost) {
                    minCost = cur->cost;
                    edge = cur;
                    bestDirection = dirInd;
                }
            }
            const auto targetDir = Normed(cars.to[i] - Center(cars.physics.rects[i]));
            const WindXy intTargetDir = {std::round(targetDir.x), std::round(targetDir.y)};
            if (!edge) {
                int dirInd = std::find(Graph::DIRS.begin(), Graph::DIRS.end(), intTargetDir) -
                    Graph::DIRS.begin();
                assert(dirInd < Graph::DIRS.size());
                edge = graph.AddEdge(vertex, dirInd);
                bestDirection = dirInd;
            }
            if (NORMAL(gen) > 0.5f) {
                std::array<int, Graph::DIRS.size()> dirInds;
                std::iota(dirInds.begin(), dirInds.end(), 0);
                std::sort(dirInds.begin(), dirInds.end(), [&](int lhs, int rhs) {
                    return Cos(Graph::DIRS[lhs], targetDir) > Cos(Graph::DIRS[rhs], targetDir);
                });
                for (int dirInd : dirInds) {
                    if (!graph.HasEdge(vertex, dirInd)) {
                        edge = graph.AddEdge(vertex, dirInd);
                        bestDirection = dirInd;
                        break;
                    }
                }
            }
            --edge->cost;
            cars.physics.SetVelocity(i, Cars::SPEED * Graph::DIRS[bestDirection]);
        }

        cars.Update(window);

        // spawn cars
        for (int i = 0; i < sources.size(); ++i) {
            if (periods[i].rest == 0) {
                cars.Add(
                    sources[i],
                    (targets[0] - sources[i]) / 100.f,
                    targets[0]);
                periods[i].rest = periods[i].total;
            }
            --periods[i].rest;
        }

        // remove cars
        for (int i = cars.physics.Size(); i >= 0; --i) {
            if (std::abs(Center(cars.physics.rects[i]) - targets[0]) < 20) {
                cars.Erase(i);
            }
        }
    }

    void Render(sf::RenderWindow& window, float part) {
        cars.Render(window, part);

        for (auto pos : sources) {
            auto shape = sf::CircleShape(30);
            shape.setPosition(WindXy(pos.x, pos.y) - Center(shape.getGlobalBounds()));
            shape.setFillColor(sf::Color::Cyan);
            window.draw(shape);
        }
        for (auto pos : targets) {
            auto shape = sf::CircleShape(30);
            shape.setPosition(WindXy(pos.x, pos.y) - Center(shape.getGlobalBounds()));
            shape.setFillColor(sf::Color::Green);
            window.draw(shape);
        }

        graph.Render(window);
    }

private:
    Cars cars;
    Graph graph = Graph((WIDTH / DPIXELS + 1) * (HEIGHT / DPIXELS + 1));
    std::vector<WindXy> sources;

    struct Period {
        int total;
        int rest = 0;
    };

    std::vector<Period> periods;
    std::vector<WindXy> targets;
};

void MainCars() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window({WIDTH, HEIGHT}, "Game", sf::Style::Default, settings);
    Game game;

    sf::Clock clock;
    sf::Int64 lag = 0;
    const int usPerUpdate = 300;

    while (window.isOpen()) {
        sf::Event event;
        if (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::Closed) {
                std::exit(0);
            }
        }
        auto elapsed = clock.restart();
        lag += elapsed.asMicroseconds();

        if (lag >= usPerUpdate) {
            game.Update(window);
            lag -= usPerUpdate;
        }

        window.clear(PEACH_PUFF);
        game.Render(window, static_cast<float>(lag) / usPerUpdate);
        window.display();
    }
}

int main() {
//    Main();
//    BottleOfWater();
    MainCars();
}
