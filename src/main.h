#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

#include "utils.h"

inline std::vector<sf::Color> kCOLORS = {sf::Color::Cyan, sf::Color::Red, sf::Color::Blue, sf::Color::Green};

template <class TGame>
void Main(TGame& game, int usPerUpdate = 30000) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window({WIDTH, HEIGHT}, "TGame", sf::Style::Default, settings);

    sf::Clock clock;
    sf::Int64 lag = 0;

    while (window.isOpen()) {
        sf::Event event;
        if (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::Closed) {
                std::exit(0);
            }
            game.HandleInput(event);
        }
        auto elapsed = clock.restart().asMicroseconds();
        lag += elapsed;
        gStats["elapsed, mcs"] = elapsed;

        if (lag >= usPerUpdate) {
            gStats["CheckCollision calls"] = 0;
            gStats["CheckCollision checks"] = 0;
            game.Update(window);
            ++gStats["updates"];
            lag -= usPerUpdate;
        }

        window.clear(PEACH_PUFF);
        if (lag < 1) {
            game.Render(window, static_cast<float>(lag) / usPerUpdate);
        } else {
            game.Render(window, 0);
        }
        ++gStats["ticks"];
//        if (gStats["ticks"] == 359) {
//            std::cout << "Here" << std::endl;
//        }
        DrawStats(window);
        window.display();
    }
}
