#pragma once

#include "utils.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

template <class T, class U>
inline auto To(const sf::Vector2<U>& other) {
    return sf::Vector2<T>(other.x, other.y);
}

struct WaterBottle {
    WaterBottle() {
        auto& left = borders_.emplace_back(WindXy(50, 500));
        left.setPosition(WindXy(200, 200));
        auto& bot = borders_.emplace_back(WindXy(1100, 50));
        bot.setPosition(WindXy(200, 700));
        auto& top = borders_.emplace_back(WindXy(1100, 50));
        top.setPosition(WindXy(200, 150));
        auto& right = borders_.emplace_back(WindXy(50, 500));
        right.setPosition(WindXy(1250, 200));
        for (auto& border : borders_) {
            border.setFillColor(GREY);
        }

        int ySize = (right.getPosition().x - left.getPosition().x - left.getSize().x) / ds_;
        int xSize = (bot.getPosition().y - right.getPosition().y) / ds_;
        density_.resize(xSize);
        for (auto& row : density_) {
            row.resize(ySize);
            std::generate(row.begin(), row.end(), []() {
                std::uniform_real_distribution dis;
                return dis(gen);
            });
            initialDensitySum_ += std::reduce(row.begin(), row.end());
        }
        velocity_ = {static_cast<std::size_t>(xSize), std::vector<sf::Vector2f>(ySize)};
        pressure_ = {static_cast<std::size_t>(xSize), std::vector<float>(ySize)};
        RecalcPressure();
    }

    void HandleInput(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
            canUpdate_ = true;
        }
    }

    void Update(sf::RenderWindow& window) {
        if (!canUpdate_) {
            return;
        }
//        canUpdate_ = false;
        RecalcVelocities();

        // move and calc intersections
        std::vector<std::vector<float>> masses(
            velocity_.size(),
            std::vector<float>(velocity_[0].size()));
        for (int i = 0; i < velocity_.size(); ++i) {
            for (int j = 0; j < velocity_[0].size(); ++j) {
                auto cell = sf::FloatRect(
                    origin_.x + ds_ * j,
                    origin_.y + ds_ * i,
                    ds_,
                    ds_);
                Move(cell, velocity_[i][j]);
//                if (std::abs(velocity_[i][j]) > 100) {
//                    std::cerr << "WTF!!" << std::endl;
//                }
                static constexpr std::array<std::pair<int, int>, 4> NEIGHBORS = {{
                    {0, 0},
                    {0, 1},
                    {1, 0},
                    {1, 1},
                }};

                int yIndTarget = (cell.left - origin_.x) / ds_;
                yIndTarget = std::min((int)velocity_[0].size() - 1, std::max(yIndTarget, 0));
                int xIndTarget = (cell.top - origin_.y) / ds_;
                xIndTarget = std::min((int)velocity_.size() - 1, std::max(xIndTarget, 0));
                for (auto [dx, dy] : NEIGHBORS) {
                    auto neighbor = sf::FloatRect(
                        origin_.x + ds_ * (yIndTarget + dy),
                        origin_.y + ds_ * (xIndTarget + dx),
                        ds_,
                        ds_);
                    sf::FloatRect intersection;
                    cell.intersects(neighbor, intersection);
                    const auto area = intersection.width * intersection.height;
                    int x = std::min(xIndTarget + dx, (int)velocity_.size() - 1);
                    int y = std::min(yIndTarget + dy, (int)velocity_[0].size() - 1);
                    masses[x][y] += area * density_[i][j];
                }
            }
        }
        // calc densities
        for (int i = 0; i < velocity_.size(); ++i) {
            for (int j = 0; j < velocity_[0].size(); ++j) {
                density_[i][j] = masses[i][j] / ds_ / ds_;
            }
        }

        CheckSameMass();

        RecalcPressure();
    }

    void Render(sf::RenderWindow& window, float part) const {
        for (const auto& border : borders_) {
            window.draw(border);
        }
        for (int i = 0; i < density_.size(); ++i) {
            for (int j = 0; j < density_[0].size(); ++j) {
                const auto x = origin_.x + j * ds_;
                const auto y = origin_.y + i * ds_;
                auto rect = sf::RectangleShape(WindXy(ds_, ds_));
                rect.setPosition(WindXy(x, y));
                const sf::Color cyan = {0, 255, 255, 150};
                auto color = cyan;
                color.b = std::min(static_cast<float>(cyan.b), cyan.b / 2 * density_[i][j]);
                color.g = std::min(static_cast<float>(cyan.g), cyan.g / 2 * density_[i][j]);
                rect.setFillColor(color);
                window.draw(rect);
            }
        }
    }

private:
    void CheckSameMass() {
        float densitySum = 0;
        for (const auto& row : density_) {
            densitySum += std::reduce(row.begin(), row.end());
        }
        auto diff = initialDensitySum_ - densitySum;
        if (diff > 1e-2 * std::max(std::abs(initialDensitySum_), std::abs(densitySum))) {
            std::cerr << initialDensitySum_ << " != " << densitySum << std::endl;
//            throw std::runtime_error("Densities are not the same!");
        }
        diff /= density_.size() * density_[0].size();
        for (int i = 0; i < velocity_.size(); ++i) {
            for (int j = 0; j < velocity_[0].size(); ++j) {
                density_[i][j] += diff;
            }
        }
    }

    void RecalcVelocities() {
//        if (static_cast<int>(gStats["ticks"]) % 128 == 0) {
//            std::cerr << gStats["ticks"] << std::endl;
//        }
        // By Ox:
        for (int i = 0; i < velocity_.size(); ++i) {
            for (int j = 0; j < velocity_[0].size(); ++j) {
                float pressure = 0;
                if (j > 0) {
                    pressure += pressure_[i][j - 1];
                }
                if (j + 1 < velocity_[0].size()) {
                    pressure -= pressure_[i][j + 1];
                }
                float xAcc = pressure / density_[i][j] / ds_;
                velocity_[i][j] += sf::Vector2f{xAcc, 0};
            }
        }
        // By Oy:
        for (int i = 0; i < velocity_.size(); ++i) {
            for (int j = 0; j < velocity_[0].size(); ++j) {
                float pressure = 0;
                if (i > 0) {
                    pressure += pressure_[i - 1][j];
                }
                if (i + 1 < velocity_.size()) {
                    pressure -= pressure_[i + 1][j];
                }
                float yAcc = pressure / density_[i][j] / ds_;
                velocity_[i][j] += sf::Vector2f{0, yAcc};
            }
        }
    }

    void RecalcPressure() {
        for (int j = 0; j < pressure_[0].size(); ++j) {
            pressure_[0][j] = density_[0][j] * GRAVITY_CONST * ds_;
        }
        for (int i = 1; i < pressure_.size(); ++i) {
            for (int j = 0; j < pressure_[0].size(); ++j) {
                pressure_[i][j] = pressure_[i - 1][j] + density_[i][j] * GRAVITY_CONST * ds_;
            }
        }
    }

    const int ds_ = 10;
    const sf::Vector2i origin_ = {250, 200};

    std::vector<sf::RectangleShape> borders_;
    std::vector<std::vector<sf::Vector2f>> velocity_;
    std::vector<std::vector<float>> density_;
    std::vector<std::vector<float>> pressure_;
    float initialDensitySum_ = 0;

    bool canUpdate_ = false;
};
