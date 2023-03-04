#include "main.h"
#include "utils.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

#include <cassert>
#include <array>
#include <thread>
#include <experimental/forward_list>
#include <deque>
#include <unordered_set>

sf::Color TransparentBlack() {
    auto color = sf::Color::Black;
    color.a = 150;
    return color;
}

class Miner {
    enum CellContent {
        Nothing = 0,
        Mine = 1,
        Flag = 2,
        RandomFlag = 4,
        Opened = 8,
    };

    struct Cell {
        WindXy pos;
        sf::Vector2i indPos;
        int mines = 0;
        uint32_t content = 0;
    };

    enum class State {
        NotStarted,
        Running,
        Lost,
        Win,
    };

    static constexpr inline float CELL_SIZE = 150;

public:
    Miner(std::size_t rows, std::size_t cols, double prob) : prob_(prob) {
        grid_.reserve(rows);
        for (std::size_t i = 0; i < rows; ++i) {
            auto& row = grid_.emplace_back();
            for (std::size_t j = 0; j < cols; ++j) {
                row.push_back(Cell{
                    {j * CELL_SIZE, i * CELL_SIZE},
                    {static_cast<int>(i), static_cast<int>(j)},
                });
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
                if ((cell.content & CellContent::Opened) == 0 && dis(gen) < prob_) {
                    cell.content |= CellContent::Mine;
                }
            }
        }
        CalcDigits();
    }

    void CalcDigits() {
        for (auto& row : grid_) {
            for (auto& cell : row) {
                int mines = 0;
                TraverseNeighbors(cell, [&](Cell& neighbor) {
                    mines += neighbor.content & CellContent::Mine;
                });
                grid_[cell.indPos.x][cell.indPos.y].mines = mines;
            }
        }
    }

    void HandleInput(const sf::Event& event) {
        if (state_ == State::Lost || state_ == State::Win) {
            return;
        }
        if (event.type == sf::Event::EventType::MouseButtonPressed) {
            const int i = event.mouseButton.y / CELL_SIZE;
            const int j = event.mouseButton.x / CELL_SIZE;
            if (event.mouseButton.button == sf::Mouse::Button::Left) {
                Put(i, j, CellContent::Opened);
            } else if (event.mouseButton.button == sf::Mouse::Button::Right) {
                Put(i, j, CellContent::Flag);
            } else if (event.mouseButton.button == sf::Mouse::Button::Middle) {
                showMines_ = true;
            }
        }
    }

    void Update(sf::RenderWindow& window) {
        CheckFinish();
        if (state_ == State::Lost || state_ == State::Win) {
            Finish();
            return;
        }
    }

    void Render(sf::RenderWindow& window, float part) {
        sf::CircleShape mine(CELL_SIZE * 3 / 8);
        mine.setFillColor(sf::Color::Red);

        static constexpr float DIFF = 5;
        sf::RectangleShape cellShape({CELL_SIZE - 2 * DIFF, CELL_SIZE - 2 * DIFF});
        cellShape.setFillColor(utils::DARK_GREY);

        sf::RectangleShape emptyShape({CELL_SIZE - 2 * DIFF, CELL_SIZE - 2 * DIFF});
        emptyShape.setFillColor(utils::LIGHT_GREY);

        for (const auto& row : grid_) {
            for (const auto& cell : row) {
                cellShape.setPosition(cell.pos + WindXy{DIFF, DIFF});
                window.draw(cellShape);

                if (cell.content & CellContent::Flag) {
                    utils::DisplayText(
                        window,
                        cell.pos + WindXy{CELL_SIZE / 4, 0},
                        "F",
                        sf::Color::Blue,
                        CELL_SIZE * 6 / 8);
                }
                if (cell.content & CellContent::Opened) {
                    if ((cell.content & CellContent::Mine) == 0) {
                        if (cell.mines == 0) {
                            emptyShape.setPosition(cell.pos + WindXy{DIFF, DIFF});
                            window.draw(emptyShape);
                        } else {
                            utils::DisplayText(
                                window,
                                cell.pos + WindXy{CELL_SIZE / 4, 0},
                                std::to_string(cell.mines),
                                sf::Color::Black,
                                CELL_SIZE * 6 / 8);
                        }
                    } else {
                        mine.setPosition(cell.pos + WindXy{CELL_SIZE / 8, CELL_SIZE / 8});
                        mine.setFillColor(sf::Color::Black);
                        window.draw(mine);
                        mine.setFillColor(sf::Color::Red);
                    }
                } else if (cell.content & CellContent::Mine && showMines_) {
                    mine.setPosition(cell.pos + WindXy{CELL_SIZE / 8, CELL_SIZE / 8});
                    window.draw(mine);
                } else if (cell.mines > 0 && showMines_) {
                    utils::DisplayText(
                        window,
                        cell.pos + WindXy{CELL_SIZE / 4, 0},
                        std::to_string(cell.mines),
                        sf::Color::Black,
                        CELL_SIZE * 6 / 8);
                }
            }
        }
        if (state_ == State::Win) {
            showMines_ = true;
            utils::DisplayText(
                window,
                WindXy(window.getSize().x / 10, window.getSize().y / 10),
                "WIN!",
                TransparentBlack(),
                window.getSize().x * 8 / 30);
        } else if (state_ == State::Lost) {
            showMines_ = true;
            utils::DisplayText(
                window,
                WindXy(window.getSize().x / 10, window.getSize().y / 10),
                "LOST",
                TransparentBlack(),
                window.getSize().x * 8 / 30);
        }
    }

private:
    friend class Solver;

    void CheckFinish() {
        if (correctFlags_ + opened_ == grid_.size() * grid_[0].size()) {
            state_ = State::Win;
        }
    }

    void Finish() {

    }

//    bool Finished() const {
//    }

    void Put(std::size_t i, std::size_t j, CellContent content) {
        auto& cell = grid_.at(i).at(j);
        if (state_ == State::NotStarted) [[unlikely]] {
            state_ = State::Running;
            cell.content |= CellContent::Opened;
            ++opened_;
            TraverseNeighbors(cell, [this](Cell& neighbor) {
                neighbor.content |= CellContent::Opened;
                ++opened_;
            });
            Generate();
            Open(cell);
        } else if (content == CellContent::Opened) {
            if ((cell.content & CellContent::Mine) == 0) {
                int flags = 0;
                TraverseNeighbors(cell, [&flags](Cell& neighbor) {
                    if (neighbor.content & CellContent::Flag) {
                        ++flags;
                    }
                });
                if (cell.content & CellContent::Opened && cell.mines == flags) {
                    TraverseNeighbors(cell, [this](Cell& neighbor) {
                        if (std::popcount(neighbor.content & (CellContent::Mine | CellContent::Flag)) == 1) {
                            state_ = State::Lost;
                            return;
                        }
                        if ((neighbor.content & CellContent::Flag)  == 0) {
                            Open(neighbor);
                        }
                    });
                }
                Open(cell);
                return;
            }
            cell.content |= CellContent::Opened;
            state_ = State::Lost;
        } else if (content == CellContent::Flag) {
            cell.content ^= CellContent::Flag;
            if (cell.content & CellContent::Mine) {
                if (cell.content & CellContent::Flag) {
                    ++correctFlags_;
                } else {
                    --correctFlags_;
                }
            }
        }
    }

    void Open(Cell& cell) {
        std::deque<Cell*> cells{&cell};
        std::unordered_set<Cell*> used;
        while (!cells.empty()) {
            auto cur = cells.front();
            cells.pop_front();

            if (used.contains(cur)) {
                continue;
            }
            used.insert(cur);

            opened_ += (cur->content & CellContent::Opened) == 0;
            cur->content |= CellContent::Opened;
            if (cur->mines == 0) {
                TraverseNeighbors(*cur, [&cells](Cell& neighbor) {
                    cells.push_back(&neighbor);
                });
            }
        }
    }

    template <class F>
    void TraverseNeighbors(const Cell& cell, F&& f) {
        const auto [i, j] = cell.indPos;
        for (auto [dx, dy] : utils::NEIGHBORS) {
            if (i + dx >= 0 && i + dx < grid_.size()
                && j + dy >= 0 && j + dy < grid_.front().size())
            {
                f(grid_[i + dx][j + dy]);
            }
        }
    }

    std::vector<std::vector<Cell>> grid_;
    double prob_;
    State state_ = State::NotStarted;
    bool showMines_ = false;
    std::size_t opened_ = 0;
    std::size_t correctFlags_ = 0;
};

class Solver {
public:
    Solver(Miner& miner) : miner_(miner) {
    }

    void HandleInput(const sf::Event& event) {
    }

    void Update(sf::RenderWindow& window) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (step < miner_.grid_.size()) {
            miner_.Put(0, step++, Miner::CellContent::Opened);
        }
        miner_.showMines_ = true;
    }

    void Render(sf::RenderWindow& window, float part) {
        miner_.Render(window, part);
    }

private:
    Miner& miner_;
    std::size_t step = 0;
};

int main() {
    Miner miner(5, 5, 0.2);
//    Solver solver(miner);
    Main(miner, 1);
}
