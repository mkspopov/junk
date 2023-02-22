#pragma once

#include "graph.h"
#include "particles.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <cassert>
#include <vector>

struct Cars {
    void Add(WindXy at, sf::Vector2f velocity, WindXy finish) {
        sf::FloatRect rect = {at, {30, 30}};
        for (const auto& other : physics.shapes) {
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
                Move(physics.shapes[i], physics.velocities[i] * dt);
            }
        }

        for (std::size_t i : hitIndices) {
            // Cancel above Move
            Move(physics.shapes[i], -physics.velocities[i] * dt);

//            const auto j = firstHits[i]->otherIndex;
//            physics.SetVelocity(i, {0, 0});
//            physics.SetVelocity(j, {0, 0});
        }
    }

    void Erase(std::size_t index) {
        physics.Erase(index);
        to.erase(to.begin() + index);
    }

    void Render(sf::RenderWindow& window, float part) const {
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            auto rect = physics.shapes[i];
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
                Move(physics.shapes[i], physics.velocities[i] * timeLeft);
            }
        }

        std::vector<std::size_t> dead;
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            const auto& rect = physics.shapes[i];
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
            const auto vertex = GetVertex(Center(cars.physics.shapes[i]));
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
            const auto targetDir = Normed(cars.to[i] - Center(cars.physics.shapes[i]));
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
            if (std::abs(Center(cars.physics.shapes[i]) - targets[0]) < 20) {
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

void RunCarsGame() {
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
