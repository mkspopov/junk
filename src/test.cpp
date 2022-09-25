#include "particles.h"

#include <gtest/gtest.h>

TEST(Collisions, OneDimension) {
    Particles particles;
    auto check = [&](int i, sf::Vector2f pos, sf::Vector2f velocity) {
        ASSERT_FLOAT_EQ(particles.physics.rects[i].left, pos.x);
        ASSERT_FLOAT_EQ(particles.physics.rects[i].top, pos.y);
        ASSERT_FLOAT_EQ(particles.physics.velocities[i].x, velocity.x);
        ASSERT_FLOAT_EQ(particles.physics.velocities[i].y, velocity.y);
    };
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test");
    particles.Add({5, 15}, {0, 0}, {3, 0}, {5, 5});
    particles.Add({20, 17}, {0, 0}, {-2, 0}, {5, 5});
    particles.Update(window, {});
    check(0, {8, 15}, {3, 0});
    check(1, {18, 17}, {-2, 0});
    particles.Update(window, {});
    check(0, {11, 15}, {3, 0});
    check(1, {16, 17}, {-2, 0});
    particles.Update(window, {});
    check(0, {9, 15}, {-2, 0});
    check(1, {19, 17}, {3, 0});
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
