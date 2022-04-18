#include "particles.h"

#include <atomic>
#include <thread>
#include <unordered_set>

void Particles::Add(int num, sf::FloatRect where) {
    for (int i = 0; i < num;) {
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

    firstHits_.emplace_back();
    return true;
}

bool Particles::Add(float atx, float aty, float vx, float vy, float sx, float sy, float density) {
    return Add(WindXy(atx, aty), sf::Vector2f(vx, vy), sf::Vector2f(sx, sy), density);
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
    return v1 * (m1 - m2) / (m1 + m2) + 2 * m2 * v2 / (m1 + m2);
}

void Particles::Update(sf::RenderWindow& window, const std::vector<Physics*>& others) {
    for (auto other : others) {
        for (std::size_t i = 0; i < other->shapes.size(); ++i) {
            physics.PushBack(other->shapes[i], other->velocities[i], other->masses[i]);
            firstHits_.emplace_back();
        }
    }

    const std::size_t numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    int count = physics.shapes.size() / numThreads + 1;
    for (std::size_t threadInd = 0; threadInd < numThreads; ++threadInd) {
        threads.emplace_back([&, threadInd = threadInd]() {
            for (std::size_t i = threadInd * count;
                i < std::min(physics.shapes.size(), (threadInd + 1) * count);
                ++i)
            {
                for (std::size_t j = i + 1; j < physics.shapes.size(); ++j) {
                    if (!firstHits_[i]->CanHit(j)) {
                        continue;
                    }

                    auto iPos = physics.shapes[i].getPosition();
                    auto jPos = physics.shapes[j].getPosition();
                    auto velocity = physics.velocities[i] - physics.velocities[j];
//                    velocity *= dt;
                    char startCoord = 'x';

                    float start = 0;
                    float end = 1;

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
                        {
                            std::unique_lock lock(firstHits_[i]);
                            if (firstHits_[i]->time > start) {
                                firstHits_[i]->time = start;
                                firstHits_[i]->coord = startCoord;
                                firstHits_[i]->otherIndex = j;
                            }
                        }
                        {
                            std::unique_lock lock(firstHits_[j]);
                            if (firstHits_[j]->time > start) {
                                firstHits_[j]->time = start;
                                firstHits_[j]->coord = startCoord;
                                firstHits_[j]->otherIndex = i;
                            }
                        }
                    }
                }
            }
        });
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::vector<std::size_t> hitIndices;
    float minHitStart = 1;
    for (std::size_t i = 0; i < physics.shapes.size(); ++i) {
        const auto otherIndex = firstHits_[i]->otherIndex;
        if (otherIndex < physics.shapes.size() && firstHits_[otherIndex]->otherIndex == i) {
            if (minHitStart > firstHits_[i]->time) {
                minHitStart = firstHits_[i]->time;
                if (minHitStart == 0) {
//                    std::cerr << "found " << i << ' ' << otherIndex << std::endl;
//                    std::cerr << physics.shapes[i].getPosition().x <<
//                        ", " << physics.shapes[i].getPosition().y <<
//                        ", " << physics.velocities[i].x <<
//                        ", " << physics.velocities[i].y <<
//                        ", " << physics.shapes[i].getSize().x <<
//                        ", " << physics.shapes[i].getSize().y << std::endl;
//                    std::cerr << physics.shapes[otherIndex].getPosition().x <<
//                        ", " << physics.shapes[otherIndex].getPosition().y <<
//                        ", " << physics.velocities[otherIndex].x <<
//                        ", " << physics.velocities[otherIndex].y <<
//                        ", " << physics.shapes[otherIndex].getSize().x <<
//                        ", " << physics.shapes[otherIndex].getSize().y << std::endl;
                }
                hitIndices = {i};
            } else if (minHitStart == firstHits_[i]->time && i < otherIndex) {
                hitIndices.push_back(i);
            }
        }
    }
    if (minHitStart > 0) {
        for (std::size_t i = 0; i < physics.shapes.size(); ++i) {
            firstHits_[i]->cannotHitSet.clear();
        }
    }

    // Move all before the first hit
    for (std::size_t i = 0; i < physics.shapes.size(); ++i) {
        physics.shapes[i].move(physics.velocities[i] * minHitStart);
    }

    for (std::size_t i : hitIndices) {
        const auto j = firstHits_[i]->otherIndex;
        firstHits_[i]->cannotHitSet.insert(j);

        const auto iVelocity = physics.velocities[i];
        const auto jVelocity = physics.velocities[j];

        if (firstHits_[i]->coord == 'x') {
            physics.velocities[i].x = CalcElasticCollisionSpeed(physics.masses[i],
                physics.masses[j], iVelocity.x, jVelocity.x);
            physics.velocities[j].x = CalcElasticCollisionSpeed(physics.masses[j],
                physics.masses[i], jVelocity.x, iVelocity.x);
        } else {
            physics.velocities[i].y = CalcElasticCollisionSpeed(physics.masses[i],
                physics.masses[j], iVelocity.y, jVelocity.y);
            physics.velocities[j].y = CalcElasticCollisionSpeed(physics.masses[j],
                physics.masses[i], jVelocity.y, iVelocity.y);
        }
    }

    // Move all after the hit
//    for (std::size_t i = 0; i < physics.shapes.size(); ++i) {
//        physics.shapes[i].move(physics.velocities[i] * 1e-3f);
//    }

    for (auto& firstHit : firstHits_) {
        firstHit->time = 1;
        firstHit->otherIndex = - 1;
    }

//    std::cerr << minHitStart << std::endl;

    for (auto other : others) {
        for (std::size_t i = 0; i < other->shapes.size(); ++i) {
            physics.PopBack();
            firstHits_.pop_back();
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
            shape.getPosition().y + shape.getSize().y < -windY) {
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
