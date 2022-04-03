#include "particles.h"

#include <atomic>
#include <ranges>
#include <thread>
#include <unordered_set>

void Particles::Add(int num, sf::FloatRect where) {
    for (int i = 0; i < num; ) {
        WindXy pos{
            Rand(where.left, where.left + where.width),
            Rand(where.top, where.top + where.height),
        };
        if (Add(pos, {Rand(-3, 3), Rand(-3, 3)}, {Rand(20, 30), Rand(20, 30)})) {
            ++i;
        }
    }
}

bool Particles::Add(WindXy at, sf::Vector2f velocity, sf::Vector2f size, float density) {
    sf::RectangleShape shape(size);
    shape.setPosition(at);
    for (const auto& other : physics.shapes) {
        if (shape.getGlobalBounds().intersects(other.getGlobalBounds())) {
            return false;
        }
    }
    shape.setFillColor(LIGHT_GREY);
    physics.shapes.push_back(std::move(shape));
    physics.velocities.push_back(velocity);
    physics.masses.push_back(density * size.x * size.y);
    return true;
}

void Particles::Render(sf::RenderWindow& window, float part) {
    for (std::size_t i = 0; i < physics.shapes.size(); ++i) {
        auto shape = physics.shapes[i];
        shape.move(physics.velocities[i] * part);
        window.draw(shape);
    }
    decltype(toRender) toRenderNext;
    for (auto& [shape, tick] : toRender) {
        window.draw(*shape);
        --tick;
        if (tick > 0) {
            toRenderNext.emplace_back(std::move(shape), tick);
        }
    }
    toRender = std::move(toRenderNext);
}

float CalcElasticCollisionSpeed(float m1, float m2, float v1, float v2) {
//    if (v1 - v2 > 0) {
//        std::cerr << "v1" << std::endl;
//        return v1;
//    }
//    std::cerr << "huge" << std::endl;
    return v1 * (m1 - m2) / (m1 + m2) + 2 * m2 * v2 / (m1 + m2);
}

void Particles::Update(sf::RenderWindow& window, const std::vector<Physics*>& others) {
    for (auto other : others) {
        for (std::size_t i = 0; i < other->shapes.size(); ++i) {
            physics.PushBack(other->shapes[i], other->velocities[i], other->masses[i]);
        }
    }

    const int hops = 1;
    for (int hop = 0; hop < hops; ++hop) {
        const float dt = 1.0 / hops;

        const WindXy nan = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        std::vector<std::atomic<WindXy>> positionsAfterCollision(physics.shapes.size());
        for (auto& pos : positionsAfterCollision) {
            pos.store(nan, std::memory_order_release);
        }
        std::vector<std::atomic<WindXy>> velocitiesAfterCollision(physics.shapes.size());
        for (auto& vel : velocitiesAfterCollision) {
            vel.store(nan, std::memory_order_release);
        }

        const std::size_t numThreads = 32;
        std::vector<std::thread> threads;
        threads.reserve(numThreads);
        int count = physics.shapes.size() / numThreads + 1;
        for (std::size_t threadInd = 0; threadInd < numThreads; ++threadInd) {
            threads.emplace_back([&, threadInd = threadInd]() {
                for (std::size_t i = threadInd * count; i < std::min(physics.shapes.size(), (threadInd + 1) * count); ++i) {
                    for (std::size_t j = i + 1; j < physics.shapes.size(); ++j) {
                        auto iPos = physics.shapes[i].getPosition();
                        auto jPos = physics.shapes[j].getPosition();
                        auto velocity = physics.velocities[i] - physics.velocities[j];
                        velocity *= dt;
                        char startCoord = 'x';

                        float start = 0;
                        float end = dt;

                        auto check = [&](
                            float v,
                            float movingMin,
                            float movingMax,
                            float stayingMin,
                            float stayingMax,
                            char coord) {
                            if (v == 0) {
                                return stayingMin <= movingMax && movingMin <= stayingMax;
                            }
                            const auto prevStart = start;
                            if (v < 0) {
                                if (movingMax < stayingMin) {
                                    return false;
                                }
                                start = std::max(start, (stayingMax - movingMin) / v);
                                end = std::min(end, (stayingMin - movingMax) / v);
                            } else {
                                if (movingMin > stayingMax) {
                                    return false;
                                }
                                start = std::max(start, (stayingMin - movingMax) / v);
                                end = std::min(end, (stayingMax - movingMin) / v);
                            }
                            if (prevStart != start) {
                                startCoord = coord;
                            }
                            return true;
                        };

                        if (!check(
                            velocity.x,
                            iPos.x,
                            iPos.x + physics.shapes[i].getSize().x,
                            jPos.x,
                            jPos.x + physics.shapes[j].getSize().x,
                            'x')) {
                            continue;
                        }
                        if (!check(
                            velocity.y,
                            iPos.y,
                            iPos.y + physics.shapes[i].getSize().y,
                            jPos.y,
                            jPos.y + physics.shapes[j].getSize().y,
                            'y')) {
                            continue;
                        }
                        if (start <= end) {
                            auto iShape = physics.shapes[i];
                            auto jShape = physics.shapes[j];
                            auto iVelocity = physics.velocities[i];
                            auto jVelocity = physics.velocities[j];
                            iShape.move(iVelocity * start);
                            jShape.move(jVelocity * start);

                            if (startCoord == 'x') {
                                auto prevIthVel = iVelocity.x;
                                iVelocity.x = CalcElasticCollisionSpeed(physics.masses[i], physics.masses[j], prevIthVel, jVelocity.x);
                                jVelocity.x = CalcElasticCollisionSpeed(physics.masses[j], physics.masses[i], jVelocity.x, prevIthVel);
                            } else {
                                auto prevIthVel = iVelocity.y;
                                iVelocity.y = CalcElasticCollisionSpeed(physics.masses[i], physics.masses[j], prevIthVel, jVelocity.y);
                                jVelocity.y = CalcElasticCollisionSpeed(physics.masses[j], physics.masses[i], jVelocity.y, prevIthVel);
                            }

                            iShape.move(iVelocity * (dt - start));
                            jShape.move(jVelocity * (dt - start));
                            auto nanCpy = nan;
                            if (velocitiesAfterCollision[i].compare_exchange_strong(nanCpy, iVelocity)) {
                                positionsAfterCollision[i].store(iShape.getPosition(), std::memory_order_release);
                            }
                            nanCpy = nan;
                            if (velocitiesAfterCollision[j].compare_exchange_strong(nanCpy, jVelocity)) {
                                positionsAfterCollision[j].store(jShape.getPosition(), std::memory_order_release);
                            }
                        }
                    }
                    auto nanCpy = nan;
                    positionsAfterCollision[i].compare_exchange_strong(
                        nanCpy,
                        physics.shapes[i].getPosition() + physics.velocities[i] * dt);
                    nanCpy = nan;
                    velocitiesAfterCollision[i].compare_exchange_strong(
                        nanCpy,
                        physics.velocities[i]);
                }
            });
        }
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        for (std::size_t i = 0; i < physics.shapes.size(); ++i) {
            physics.shapes[i].setPosition(positionsAfterCollision[i].load(std::memory_order_acquire));
            physics.velocities[i] = velocitiesAfterCollision[i].load(std::memory_order_acquire);
        }
    }

    for (auto other : others) {
        for (std::size_t i = 0; i < other->shapes.size(); ++i) {
            physics.PopBack();
        }
    }

    std::vector<std::size_t> dead;
    for (std::size_t i = 0; i < physics.shapes.size(); ++i) {
        const auto& shape = physics.shapes[i];
        const float windX = window.getSize().x;
        const float windY = window.getSize().y;
        if (shape.getPosition().x > windX ||
            shape.getPosition().y > windY ||
            shape.getPosition().x + shape.getSize().x < -windX ||
            shape.getPosition().y + shape.getSize().y < -windY)
        {
            dead.push_back(i);
        }
    }
    // Remove dead in linear time
    for (auto i = dead.rbegin(); i != dead.rend(); ++i) {
        physics.shapes.erase(physics.shapes.begin() + *i);
        physics.velocities.erase(physics.velocities.begin() + *i);
        physics.masses.erase(physics.masses.begin() + *i);
    }
}
