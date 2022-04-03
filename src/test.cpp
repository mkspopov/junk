#include "particles.h"

#include <gtest/gtest.h>

TEST(Collisions, OneDimension) {
    Particles particles;
    auto check = [&](int i, sf::Vector2f pos, sf::Vector2f velocity) {
        ASSERT_FLOAT_EQ(particles.shapes[i].getPosition().x, pos.x);
        ASSERT_FLOAT_EQ(particles.shapes[i].getPosition().y, pos.y);
        ASSERT_FLOAT_EQ(particles.velocities[i].x, velocity.x);
        ASSERT_FLOAT_EQ(particles.velocities[i].y, velocity.y);
    };
    particles.Add({5, 15}, {3, 0}, {5, 5});
    particles.Add({20, 17}, {-2, 0}, {5, 5});
    particles.Update();
    check(0, {8, 15}, {3, 0});
    check(1, {18, 17}, {-2, 0});
    particles.Update();
    check(0, {11, 15}, {-2, 0});
    check(1, {16, 17}, {3, 0});
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
