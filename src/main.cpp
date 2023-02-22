#include "main.h"

#include "cars.h"
#include "particles.h"
#include "water.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>

using namespace std::chrono_literals;

int Main() {
    if (!PURISA_FONT.loadFromFile("/usr/share/fonts/truetype/tlwg/Purisa-Bold.ttf")) {
        throw std::runtime_error("No font!");
    }

    Particles particles;
    particles.Add({10, 10}, {0, 0}, {0, 0}, {1980, 10}, 100000);
    particles.Add({10, 20}, {0, 0}, {0, 0}, {10, 960}, 100000);
    particles.Add({10, 980}, {0, 0}, {0, 0}, {1980, 10}, 100000);
    particles.Add({1990, 10}, {0, 0}, {0, 0}, {10, 980}, 100000);

    int numParticles = 1000;
    particles.Add(numParticles, {20, 20, 1980, 980});

    sf::RenderWindow window(sf::VideoMode(2000, 1000), "Test");

    bool manualUpdate = false;
    sf::Event event;
    while (true) {
        window.clear(PEACH_PUFF);
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::KeyPressed) {
                if (event.key.code == sf::Keyboard::Key::Space) {
                    manualUpdate ^= true;
                } else if (event.key.code == sf::Keyboard::Key::Escape) {
                    std::exit(0);
                } else if (event.key.code == sf::Keyboard::Key::N && manualUpdate) {
                    particles.Update(window, {});
                }
            }
        }
        if (!manualUpdate) {
            particles.Update(window, {});
        }
        particles.Render(window, 0);
        window.display();
        std::this_thread::sleep_for(5ms);
    }
    return 0;
}

int main() {
    WaterBottle simulation;
    Main(simulation, 1);
}
