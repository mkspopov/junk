#include "main.h"

#include "cars.h"
#include "particles.h"
#include "water.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>

class CombosGame {
public:
    CombosGame(int side) : side_(side) {
    }

    void HandleInput(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Key::A) {
                dir_.x = -1;
            } else if (event.key.code == sf::Keyboard::Key::D) {
                dir_.x = 1;
            } else if (event.key.code == sf::Keyboard::Key::W) {
                dir_.y = -1;
            } else if (event.key.code == sf::Keyboard::Key::S) {
                dir_.y = 1;
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::Key::A) {
                dir_.x = 0;
            } else if (event.key.code == sf::Keyboard::Key::D) {
                dir_.x = 0;
            } else if (event.key.code == sf::Keyboard::Key::W) {
                dir_.y = 0;
            } else if (event.key.code == sf::Keyboard::Key::S) {
                dir_.y = 0;
            }
        }
    }

    void Update(sf::RenderWindow& window) {
        origin_ -= dir_;
    }

    void Render(sf::RenderWindow& window, float part) {
        for (int x = origin_.x % side_; x < static_cast<int>(window.getSize().x); x += side_) {
            sf::VertexArray line(sf::LineStrip, 2);
            line[0] = {WindXy(x, 0), sf::Color::Cyan};
            line[1] = {WindXy(x, window.getSize().y), sf::Color::Cyan};
            window.draw(line);
        }
        for (int x = origin_.y % side_; x < static_cast<int>(window.getSize().x); x += side_) {
            sf::VertexArray line(sf::LineStrip, 2);
            line[0] = {WindXy(x, 0), sf::Color::Cyan};
            line[1] = {WindXy(x, window.getSize().y), sf::Color::Cyan};
            window.draw(line);
        }
    }

private:
    sf::Vector2i origin_;
    sf::Vector2i dir_;
    sf::RectangleShape player_;
    int side_;
};

int main() {
    if (!PURISA_FONT.loadFromFile("/usr/share/fonts/truetype/tlwg/Purisa-Bold.ttf")) {
        throw std::runtime_error("No font!");
    }

//    ParticlesSimulation simulation;
//    SimpleFall simulation;
//    WaterBottle simulation;
    CombosGame simulation(100);
    Main(simulation, 5000);
}
