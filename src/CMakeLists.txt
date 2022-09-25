add_library(
    particles
    particles.cpp particles.h
)

find_package(Boost REQUIRED COMPONENTS system)
find_package(SFML REQUIRED COMPONENTS audio graphics network system window)

target_link_libraries(
    particles PUBLIC
    sfml-audio
    sfml-graphics
    sfml-network
    sfml-system
    sfml-window
    ${Boost_LIBRARIES}
)

add_library(
    hero
    hero.cpp hero.h
)

add_executable(
    main
    main.cpp
)

target_link_libraries(
    hero
    particles
)

target_link_libraries(
    main
    hero
    pthread
)

find_package(GTest REQUIRED)

add_executable(
    test_collision
    test.cpp
)

target_link_libraries(
    test_collision
    particles
    ${GTEST_LIBRARIES}
    pthread
)

add_executable(
    compile_matrices
    compile_matrices.cpp
)
