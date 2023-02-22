#include "particles.h"

#include <boost/asio.hpp>
#include <SFML/Graphics/Text.hpp>

#include <atomic>
#include <thread>
#include <unordered_set>

std::unique_ptr<boost::asio::thread_pool> threadPool;

void Particles::Add(int num, sf::FloatRect where) {
    for (int i = 0; i < num;) {
        WindXy pos{
            Rand(where.left, where.left + where.width),
            Rand(where.top, where.top + where.height),
        };
        if (Add(pos, {0, 0}, {Rand(-3, 3), Rand(-3, 3)}, {Rand(20, 30), Rand(20, 30)})) {
            ++i;
        }
    }
}

bool Particles::Add(WindXy at, sf::Vector2f acceleration, sf::Vector2f velocity, sf::Vector2f size, float density) {
    sf::FloatRect rect(at, size);
    for (const auto& other : physics.shapes) {
        if (rect.intersects(other)) {
            return false;
        }
    }
    physics.PushBack(rect, velocity, acceleration, density * size.x * size.y);

    return true;
}

bool Particles::Add(float atx, float aty, float ax, float ay, float vx, float vy, float sx, float sy, float density) {
    return Add(WindXy(atx, aty), WindXy(ax, ay), sf::Vector2f(vx, vy), sf::Vector2f(sx, sy), density);
}

void Particles::Render(sf::RenderTarget& window, float part) {
    for (std::size_t i = 0; i < physics.Size(); ++i) {
        auto shape = sf::RectangleShape({physics.shapes[i].width, physics.shapes[i].height});
        shape.setPosition(physics.shapes[i].left, physics.shapes[i].top);
        shape.move(physics.velocities[i] * part);
        shape.setFillColor(LIGHT_GREY);
        window.draw(shape);
//        DisplayText(window, shape.getPosition(), std::to_string(i), 20);
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

void SimpleMultithreaded(Particles& particles) {
//    const std::size_t numThreads = std::thread::hardware_concurrency();
//    std::vector<std::thread> threads;
//    threads.reserve(numThreads);
//    int count = physics.Size() / numThreads + 1;
//    for (std::size_t threadInd = 0; threadInd < numThreads; ++threadInd) {
//        threads.emplace_back([&, threadInd = threadInd]() {
//            for (std::size_t i = threadInd * count;
//                i < std::min(physics.Size(), (threadInd + 1) * count);
//                ++i)
//            {
//                for (std::size_t j = i + 1; j < physics.Size(); ++j) {
//                    checkCollision(i, j);
//                }
//            }
//        });
//    }
//    for (auto& thread : threads) {
//        if (thread.joinable()) {
//            thread.join();
//        }
//    }
}

float CalcElasticCollisionSpeed(float m1, float m2, float v1, float v2) {
    return v1 * (m1 - m2) / (m1 + m2) + 2 * m2 * v2 / (m1 + m2);
}

bool StrictlyIntersects(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.left + a.width > b.left
        && a.left < b.left + b.width
        && a.top + a.height > b.top
        && a.top < b.top + b.height;
}

bool StrictlyIntersects(const sf::RectangleShape& a, const sf::RectangleShape& b) {
    return StrictlyIntersects(a.getGlobalBounds(), b.getGlobalBounds());
}

void CollisionDetector::CheckCollision(Physics& physics, std::size_t i, std::size_t j) {
    ++gStats["CheckCollision calls"];
    if (!firstHits_[i]->CanHit(j)) {
        return;
    }

    ++gStats["CheckCollision checks"];
    auto iPos = GetPosition(physics.shapes[i]);
    auto jPos = GetPosition(physics.shapes[j]);
    auto velocity = physics.velocities[i] - physics.velocities[j];
    char startCoord = 'x';

    float start = 0;
    float end = 1;

    auto check = [&](
        float v,
        float movingMin,
        float movingMax,
        float stayingMin,
        float stayingMax,
        char coord)
    {
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
        if (prevStart <= start) {
            startCoord = coord;
        }
        return true;
    };

    if (!check(
        velocity.x,
        iPos.x,
        iPos.x + physics.shapes[i].width,
        jPos.x,
        jPos.x + physics.shapes[j].width,
        'x'))
    {
        return;
    }
    if (!check(
        velocity.y,
        iPos.y,
        iPos.y + physics.shapes[i].height,
        jPos.y,
        jPos.y + physics.shapes[j].height,
        'y'))
    {
        return;
    }
    if (start < end) {
        if (i < j) {
            std::unique_lock lock(firstHits_[i]);
            firstHits_[i]->AddHit(start, startCoord, j);
        } else {
            std::unique_lock lock(firstHits_[j]);
            firstHits_[j]->AddHit(start, startCoord, i);
        }
        if (StrictlyIntersects(physics.shapes[i], physics.shapes[j])) {
            std::cerr << "Found strict collision between " << i << " and " << j << std::endl;
        }
    }
};

void CollisionDetector::SimpleCheck(Physics& physics, const std::vector<BoundingBox>& boxes) {
    for (std::size_t i = 0; i < boxes.size(); ++i) {
        for (std::size_t j = i + 1; j < boxes.size(); ++j) {
            CheckCollision(physics, boxes[i].index, boxes[j].index);
        }
    }
}

void CollisionDetector::UpdateCollisions(
    Physics& physics,
    std::vector<BoundingBox>& boxes,
    bool sortByX)
{
    if (boxes.size() <= 5) {
        SimpleCheck(physics, boxes);
        return;
    }

    std::vector<BoundingBox> left, right;
    if (sortByX) {
        std::nth_element(
            boxes.begin(),
            boxes.begin() + boxes.size() / 2,
            boxes.end(),
            [](const BoundingBox& a, const BoundingBox& b) {
                return a.rect.left < b.rect.left;
            });
        auto mid = boxes[boxes.size() / 2].rect.left - 1e-3f;
        for (const auto& box : boxes) {
            if (box.rect.left > mid) {
                right.push_back(box);
            } else if (box.rect.left + box.rect.width < mid) {
                left.push_back(box);
            } else {
                left.push_back(box);
                right.push_back(box);
            }
        }
    } else {
        std::nth_element(
            boxes.begin(),
            boxes.begin() + boxes.size() / 2,
            boxes.end(),
            [](const BoundingBox& a, const BoundingBox& b) {
                return a.rect.top < b.rect.top;
            });
        auto mid = boxes[boxes.size() / 2].rect.top - 1e-5f;
        for (const auto& box : boxes) {
            if (box.rect.top > mid) {
                right.push_back(box);
            } else if (box.rect.top + box.rect.height < mid) {
                left.push_back(box);
            } else {
                left.push_back(box);
                right.push_back(box);
            }
        }
    }

    boost::asio::post(
        *threadPool,
        [
            this,
            left = std::move(left),
            size = boxes.size(),
            sortByX,
            &physics
        ]() mutable
        {
            if (left.size() < size) {
                UpdateCollisions(physics, left, !sortByX);
            } else {
                SimpleCheck(physics, left);
            }
        }
    );
    boost::asio::post(
        *threadPool,
        [
            this,
            right = std::move(right),
            size = boxes.size(),
            sortByX,
            &physics
        ]() mutable
        {
            if (right.size() < size) {
                UpdateCollisions(physics, right, !sortByX);
            } else {
                SimpleCheck(physics, right);
            }
        }
    );
}

void Particles::CollisionsCallback(
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
        for (std::size_t jInd = 0; jInd < firstHits[i]->otherIndices.size(); ++jInd) {
            const auto j = firstHits[i]->otherIndices[jInd];
            firstHits[i]->cannotHitSet.insert(j);
            firstHits[j]->cannotHitSet.insert(i);

            const auto iVelocity = physics.velocities[i];
            const auto jVelocity = physics.velocities[j];

            if (firstHits[i]->coords[jInd] == 'x') {
                physics.SetVelocity(i, sf::Vector2f{
                    CalcElasticCollisionSpeed(physics.masses[i], physics.masses[j], iVelocity.x,
                        jVelocity.x),
                    physics.velocities[i].y,
                });
                physics.SetVelocity(j, sf::Vector2f{
                    CalcElasticCollisionSpeed(physics.masses[j], physics.masses[i], jVelocity.x,
                        iVelocity.x),
                    physics.velocities[j].y,
                });
            } else {
                physics.SetVelocity(i, sf::Vector2f{
                    physics.velocities[i].x,
                    CalcElasticCollisionSpeed(physics.masses[i], physics.masses[j], iVelocity.y,
                        jVelocity.y),
                });
                physics.SetVelocity(j, sf::Vector2f{
                    physics.velocities[j].x,
                    CalcElasticCollisionSpeed(physics.masses[j], physics.masses[i], jVelocity.y,
                        iVelocity.y),
                });
            }
        }
    }
}

bool CollisionDetector::Detect(Physics& physics, float& timeLeft, TCallback callback) {
    firstHits_.resize(physics.Size());
    boxes_.clear();
    boxes_.reserve(physics.Size());

    for (auto& firstHit : firstHits_) {
        firstHit->time = 1;
        firstHit->otherIndices.clear();
        firstHit->coords.clear();
    }
    for (std::size_t i = 0; i < physics.Size(); ++i) {
        const auto [x, y] = GetPosition(physics.shapes[i]);
        const auto [vx, vy] = physics.velocities[i] * timeLeft;
        boxes_.push_back({
            {
                std::min(x, x + vx),
                std::min(y, y + vy),
                physics.shapes[i].width + std::abs(vx),
                physics.shapes[i].height + std::abs(vy),
            },
            i,
        });
    }
    threadPool = std::make_unique<boost::asio::thread_pool>();
    UpdateCollisions(physics, boxes_, true);
    threadPool->join();

    std::vector<std::size_t> hitIndices;
    float minHitStart = 1;
    for (std::size_t i = 0; i < physics.Size(); ++i) {
        if (minHitStart > firstHits_[i]->time) {
            minHitStart = firstHits_[i]->time;
            hitIndices.clear();
            hitIndices.push_back(i);
        } else if (minHitStart == firstHits_[i]->time) {
            hitIndices.push_back(i);
        }
    }
    if (minHitStart > 0) {
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            firstHits_[i]->cannotHitSet.clear();
        }

        if (minHitStart > timeLeft) {
            timeLeft = 0;
            callback(physics, timeLeft, {}, firstHits_);
            return false;
        }
    }

    timeLeft -= minHitStart;
    callback(physics, minHitStart, hitIndices, firstHits_);

    return true;
}

void CollisionDetector::Clear() {
    boxes_.clear();
    firstHits_.clear();
}

void Particles::Update(sf::RenderTarget& window, const std::vector<Physics*>& others) {
    for (std::size_t i = 0; i < physics.Size(); ++i) {
        physics.SetVelocity(i, physics.velocities[i] + physics.accelerations[i]);
    }
    float timeLeft = 1;
    for (auto other : others) {
        for (std::size_t i = 0; i < other->Size(); ++i) {
            physics.PushBack(
                other->shapes[i],
                other->accelerations[i],
                other->velocities[i],
                other->masses[i]);
        }
    }

    static CollisionDetector detector;
    detector.Clear();

    while (timeLeft > 0 && detector.Detect(physics, timeLeft, Particles::CollisionsCallback)) {
    }

    if (timeLeft > 0) {
        for (std::size_t i = 0; i < physics.Size(); ++i) {
            Move(physics.shapes[i], physics.velocities[i] * timeLeft);
        }
    }

    for (auto other : others) {
        for (std::size_t i = 0; i < other->Size(); ++i) {
            physics.PopBack();
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
    }
}

void Particles::HandleInput(const sf::Event& event) {

}
