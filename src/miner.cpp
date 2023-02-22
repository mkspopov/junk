#include "main.h"
#include "utils.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

#include <cassert>
#include <array>
#include <thread>
#include <experimental/forward_list>

class Miner {
    enum CellContent {
        Nothing = 0,
        Mine = 1,
        Flag = 2,
        RandomFlag = 4,
        Starting = 8,
    };

    struct Cell {
        WindXy pos;
        int mines = 0;
        uint32_t content = 0;
    };

    static constexpr inline float CELL_SIZE = 150;

public:
    Miner(std::size_t rows, std::size_t cols, double prob) : prob_(prob) {
        grid_.reserve(rows);
        for (std::size_t i = 0; i < rows; ++i) {
            auto& row = grid_.emplace_back();
            for (std::size_t j = 0; j < cols; ++j) {
                row.push_back({{j * CELL_SIZE, i * CELL_SIZE}});
            }
        }
    }

    void Generate() {
        if (prob_ < 0 || prob_ > 1) {
            throw std::runtime_error("bad prob");
        }
        std::mt19937 gen;
        std::uniform_real_distribution dis;
        for (auto& row : grid_) {
            for (auto& cell : row) {
//                TraverseNeighbors(i, j, [&](int i, int j) {
//                    mines += grid_[i][j].content & CellContent::Mine;
//                });
                if (dis(gen) < prob_) {
                    cell.content |= CellContent::Mine;
                }
            }
        }
    }

    void PutDigits() {
        for (int i = 0; i < grid_.size(); ++i) {
            for (int j = 0; j < grid_.front().size(); ++j) {
                int mines = 0;
                TraverseNeighbors(i, j, [&](int i, int j) {
                    mines += grid_[i][j].content & CellContent::Mine;
                });
                grid_[i][j].mines = mines;
            }
        }
    }

    void HandleInput(const sf::Event& event) {
    }

    void Update(sf::RenderWindow& window) {
        PutDigits();
    }

    void Render(sf::RenderWindow& window, float part) {
        sf::CircleShape mine(CELL_SIZE * 3 / 8);
        mine.setFillColor(sf::Color::Red);

        static constexpr float DIFF = 5;
        sf::RectangleShape cellShape({CELL_SIZE - 2 * DIFF, CELL_SIZE - 2 * DIFF});
        cellShape.setFillColor(utils::DARK_GREY);

        for (const auto& row : grid_) {
            for (const auto& cell : row) {
                cellShape.setPosition(cell.pos + WindXy{DIFF, DIFF});
                window.draw(cellShape);

                if (cell.content & CellContent::Mine) {
                    mine.setPosition(cell.pos + WindXy{CELL_SIZE / 8, CELL_SIZE / 8});
                    window.draw(mine);
                } else if (cell.content & CellContent::Flag) {
                    utils::DisplayText(
                        window,
                        cell.pos + WindXy{CELL_SIZE / 4, 0},
                        "F",
                        CELL_SIZE * 6 / 8);
                } else if (cell.mines > 0) {
                    utils::DisplayText(
                        window,
                        cell.pos + WindXy{CELL_SIZE / 4, 0},
                        std::to_string(cell.mines),
                        CELL_SIZE * 6 / 8);
                }
            }
        }
    }

private:
    friend class Solver;

    bool Put(std::size_t i, std::size_t j, CellContent content) {
        auto& cell = grid_.at(i).at(j);
        if (content == CellContent::Mine) {
            return (cell.content & CellContent::Mine) == 0;
        }
        if (started_) [[likely]] {
            cell.content |= content;
        } else {
            started_ = true;
            Generate();
        }
        return true;
    }

    template <class F>
    void TraverseNeighbors(int i, int j, F&& f) {
        for (auto [dx, dy] : utils::NEIGHBORS) {
            if (i + dx >= 0 && i + dx < grid_.size()
                && j + dy >= 0 && j + dy < grid_.front().size())
            {
                f(i + dx, j + dy);
            }
        }
    }

    std::vector<std::vector<Cell>> grid_;
    double prob_;
    bool started_ = false;
};

class Solver {
public:
    Solver(Miner& miner) : miner_(miner) {
    }

    void HandleInput(const sf::Event& event) {
    }

    void Update(sf::RenderWindow& window) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void Render(sf::RenderWindow& window, float part) {
        miner_.Render(window, part);
    }

private:
    Miner& miner_;
};

int main() {
    Miner miner(10, 10, 0.2);
    Solver solver(miner);
    Main(solver, 1);
}
