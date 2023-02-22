#include "grid_collision.h"

#include "main.h"

class WaterSimulation {
public:
    WaterSimulation(std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            sf::CircleShape shape(SIZE / 2);
            const std::size_t dist = (4 * SIZE);
            const std::size_t inRow = WIDTH / dist;
            shape.setPosition((i % inRow) * dist, i / inRow * dist);
            shape.setFillColor(DARK_GREY);
            particles_.PushBack(std::move(shape), RandVec(-1.f, 1.f), {}, 1);
        }
    }

    void HandleInput(const sf::Event& event) {
    }

    void Update(sf::RenderWindow& window) {
        struct Cell {
            struct Drop {
                sf::CircleShape shape;
                sf::Vector2f* velocity;
            };
            std::vector<Drop> drops;
//            std::vector<std::size_t> ids;
        };
//        const std::size_t cellSide = std::sqrt(WIDTH * HEIGHT / particles_.Size());
        const std::size_t cellSide = 30;
        std::vector<std::vector<Cell>> grid(HEIGHT / cellSide, std::vector<Cell>(WIDTH / cellSide));
        for (std::size_t i = 0; i < particles_.Size(); ++i) {
            auto [x, y] = particles_.shapes[i].getPosition();
            if (x < 0 || y < 0) {
                continue;
            }
            const std::size_t xInd = x / cellSide;
            const std::size_t yInd = y / cellSide;
            if (yInd < grid.size() && xInd < grid[0].size()) {
                grid[xInd][yInd].drops.push_back({particles_.shapes[i], &particles_.velocities[i]});
//                grid[xInd][yInd].ids.push_back(i);
            }
        }

        for (std::size_t row = 0; row < grid.size(); ++row) {
            for (std::size_t col = 0; col < grid[0].size(); ++col) {

                for (std::size_t i = 0; i < grid[row][col].drops.size(); ++i) {
                    for (std::size_t j = i + 1; j < grid[row][col].drops.size(); ++j) {
                        auto& first = grid[row][col].drops[i];
                        auto& second = grid[row][col].drops[j];

                        auto v = *first.velocity - *second.velocity;
                        auto cross = std::abs(CrossProduct(v, Center(second.shape) - Center(first.shape)));
                        auto h = cross / std::abs(v);

                        if (h < 2 * first.shape.getRadius()) {
                            // collision hit, simply swap velocities
                            std::swap(*first.velocity, *second.velocity);
                        }
                    }
                }

            }
        }

        for (std::size_t i = 0; i < particles_.Size(); ++i) {
            particles_.velocities[i].y += GRAVITY_CONST;
            if (particles_.shapes[i].getPosition().y > HEIGHT - 2 * particles_.shapes[0].getRadius()) {
                particles_.velocities[i].y = -2 * particles_.velocities[i].y / 3;
            }
            particles_.shapes[i].move(particles_.velocities[i]);
        }
    }

    void Render(sf::RenderWindow& window, float part) {
        for (const auto& shape : particles_.shapes) {
            window.draw(shape);
        }
    }

private:
    static constexpr inline float SIZE = 20;

    Physics<sf::CircleShape> particles_;
    grid::GridCollision grid_{particles_, SIZE};
};

int main() {
    WaterSimulation simulation(10000);
    Main(simulation);
}
