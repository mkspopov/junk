
struct ParticlesSimulation {
    ParticlesSimulation();

    void HandleInput(const sf::Event& event) {
        if (event.type == sf::Event::EventType::Closed) {
            std::exit(0);
        } else if (event.type == sf::Event::EventType::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Button::Left) {
                spammingBricks = true;
            }
        } else if (event.type == sf::Event::EventType::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Button::Left) {
                spammingBricks = false;
            }
        }
    }

    void Render(sf::RenderWindow& window, float part) {
        particles.Render(window, part);
        earthGravity.Render(window);
    }

    void Update(sf::RenderWindow& window) {
        if (spammingBricks) {
            auto [x, y] = sf::Mouse::getPosition(window);
            particles.Add(
                WindXy(x, y),
                {0, 0},
                sf::Vector2f(Rand(-0.5, 0.5), Rand(-0.5, 0.5)),
                sf::Vector2f(Rand(10, 20), Rand(10, 20))
            );
        }

        earthGravity.Apply(particles.physics);
        particles.Update(window, {});
//        texture.getTexture().copyToImage().saveToFile("screenshot_" + std::to_string(tick) + ".png");
    }

    Particles particles;
    GravityForce earthGravity = {
        1e11,
        {WIDTH / 2, 1'000'000},
    };
    bool spammingBricks = false;
};

ParticlesSimulation::ParticlesSimulation() {
    particles.Add(WindXy(200, 200), {0, 0}, {0, 0}, WindXy(50, 500));
    particles.physics.properties.back().reset(Physics::Properties::Move);
    particles.physics.properties.back().reset(Physics::Properties::Gravity);
    particles.Add(WindXy(200, 700), {0, 0}, {0, 0}, WindXy(1000, 50));
    particles.physics.properties.back().reset(Physics::Properties::Move);
    particles.physics.properties.back().reset(Physics::Properties::Gravity);
    particles.Add(WindXy(1150, 200), {0, 0}, {0, 0}, WindXy(50, 500));
    particles.physics.properties.back().reset(Physics::Properties::Move);
    particles.physics.properties.back().reset(Physics::Properties::Gravity);
//    particles.Add({0, HEIGHT - 100}, {0, 0}, {0, 0}, {WIDTH, 50}, 1000000);
    for (int i = 0; i < 30; ++i) {
        particles.Add(
            {Rand(0, WIDTH), Rand(0, HEIGHT)},
            {0, 0},
            {0, 0},
            sf::Vector2f(Rand(10, 20), Rand(10, 20))
        );
    }
}

struct SimpleFall {
    SimpleFall() {
        particles.Add({100, 460}, {0, 0}, {0, 0}, {30, 30});
        particles.Add({200, 462}, {0, 0}, {0, 0}, {30, 30});
        particles.Add({300, 463}, {0, 0}, {0, 0}, {30, 30});
        particles.Add({400, 464}, {0, 0}, {0, 0}, {30, 30});
        particles.Add({500, 465}, {0, 0}, {0, 0}, {30, 30});
        particles.Add({0, 500}, {0, 0}, {0, 0}, {1000, 100});
        particles.physics.properties.back().reset(Physics::Properties::Move);
    }

    void HandleInput(const sf::Event& event) {
    }

    void Update(sf::RenderWindow& window) {
        particles.physics.velocities[0] += {0, 2};
        particles.physics.velocities[1] += {0, 2};
        particles.physics.velocities[2] += {0, 2};
        particles.physics.velocities[3] += {0, 2};
        particles.physics.velocities[4] += {0, 2};

        particles.physics.velocities[5] = {0, 0};

        particles.Update(window, {});
    }

    void Render(sf::RenderWindow& window, float part) {
        particles.Render(window, part);
    }

    Particles particles;
};
