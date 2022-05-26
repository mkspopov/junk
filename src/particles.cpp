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
    for (const auto& other : physics.rects) {
        if (rect.intersects(other)) {
            return false;
        }
    }
    physics.rects.push_back(std::move(rect));
    physics.accelerations.push_back(acceleration);
    physics.velocities.push_back(velocity);
    physics.masses.push_back(density * size.x * size.y);

    return true;
}

bool Particles::Add(float atx, float aty, float ax, float ay, float vx, float vy, float sx, float sy, float density) {
    return Add(WindXy(atx, aty), WindXy(ax, ay), sf::Vector2f(vx, vy), sf::Vector2f(sx, sy), density);
}

void DisplayText(sf::RenderWindow& window, WindXy position, const std::string& str, uint size = 10) {
    sf::Text text(str, PURISA_FONT);
    text.setCharacterSize(size);
    text.setStyle(sf::Text::Bold);
    text.setFillColor(sf::Color::Black);
    text.setString(str);
    text.setPosition(position);
    window.draw(text);
}

void Particles::Render(sf::RenderWindow& window, float part) {
    for (std::size_t i = 0; i < physics.Size(); ++i) {
        auto shape = sf::RectangleShape({physics.rects[i].width, physics.rects[i].height});
        shape.setPosition(physics.rects[i].left, physics.rects[i].top);
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
    if (!firstHits_[i]->CanHit(j)) {
        return;
    }

    auto iPos = GetPosition(physics.rects[i]);
    auto jPos = GetPosition(physics.rects[j]);
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
        if (prevStart != start) {
            startCoord = coord;
        }
        return true;
    };

    if (!check(
        velocity.x,
        iPos.x,
        iPos.x + physics.rects[i].width,
        jPos.x,
        jPos.x + physics.rects[j].width,
        'x'))
    {
        return;
    }
    if (!check(
        velocity.y,
        iPos.y,
        iPos.y + physics.rects[i].height,
        jPos.y,
        jPos.y + physics.rects[j].height,
        'y'))
    {
        return;
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
        if (StrictlyIntersects(physics.rects[i], physics.rects[j])) {
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
            Move(physics.rects[i], physics.velocities[i] * dt);
        }
    }

    for (std::size_t i : hitIndices) {
        const auto j = firstHits[i]->otherIndex;
        firstHits[i]->cannotHitSet.insert(j);
        firstHits[j]->cannotHitSet.insert(i);

        const auto iVelocity = physics.velocities[i];
        const auto jVelocity = physics.velocities[j];

        if (firstHits[i]->coord == 'x') {
            physics.SetVelocity(i, sf::Vector2f{
                CalcElasticCollisionSpeed(physics.masses[i], physics.masses[j], iVelocity.x, jVelocity.x),
                physics.velocities[i].y,
            });
            physics.SetVelocity(j, sf::Vector2f{
                CalcElasticCollisionSpeed(physics.masses[j], physics.masses[i], jVelocity.x, iVelocity.x),
                physics.velocities[j].y,
            });
        } else {
            physics.SetVelocity(i, sf::Vector2f{
                physics.velocities[i].x,
                CalcElasticCollisionSpeed(physics.masses[i], physics.masses[j], iVelocity.y, jVelocity.y),
            });
            physics.SetVelocity(j, sf::Vector2f{
                physics.velocities[j].x,
                CalcElasticCollisionSpeed(physics.masses[j], physics.masses[i], jVelocity.y, iVelocity.y),
            });
        }
    }
}

bool CollisionDetector::Detect(Physics& physics, float& timeLeft, TCallback callback) {
    firstHits_.resize(physics.Size());
    boxes_.clear();
    boxes_.reserve(physics.Size());

    for (auto& firstHit : firstHits_) {
        firstHit->time = 1;
        firstHit->otherIndex = -1;
    }
    for (std::size_t i = 0; i < physics.Size(); ++i) {
        const auto [x, y] = GetPosition(physics.rects[i]);
        const auto [vx, vy] = physics.velocities[i] * timeLeft;
        boxes_.push_back({
            {
                std::min(x, x + vx),
                std::min(y, y + vy),
                physics.rects[i].width + std::abs(vx),
                physics.rects[i].height + std::abs(vy),
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
        const auto otherIndex = firstHits_[i]->otherIndex;
        if (i < otherIndex &&
            otherIndex < physics.Size() &&
            firstHits_[otherIndex]->otherIndex == i) {
            if (minHitStart > firstHits_[i]->time) {
                minHitStart = firstHits_[i]->time;
                hitIndices = {i};
            } else if (minHitStart == firstHits_[i]->time) {
                hitIndices.push_back(i);
            }
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

void Particles::Update(sf::RenderWindow& window, const std::vector<Physics*>& others) {
    for (std::size_t i = 0; i < physics.Size(); ++i) {
        physics.SetVelocity(i, physics.velocities[i] + physics.accelerations[i]);
    }
    float timeLeft = 1;
    for (auto other : others) {
        for (std::size_t i = 0; i < other->Size(); ++i) {
            physics.PushBack(
                other->rects[i],
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
            Move(physics.rects[i], physics.velocities[i] * timeLeft);
        }
    }

    for (auto other : others) {
        for (std::size_t i = 0; i < other->Size(); ++i) {
            physics.PopBack();
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
    }
}
