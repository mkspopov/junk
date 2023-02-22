add_library(
    particles
    particles.cpp particles.h
)

find_package(Boost REQUIRED COMPONENTS system)
find_package(SFML REQUIRED COMPONENTS audio graphics network system window)

set(
    EXTERNAL_LIBRARIES
    sfml-audio
    sfml-graphics
    sfml-network
    sfml-system
    sfml-window
    ${Boost_LIBRARIES}
)

target_link_libraries(
    particles PUBLIC
    ${EXTERNAL_LIBRARIES}
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

add_executable(
    matrix
    matrix.cpp
)

target_compile_options(matrix PRIVATE -march=native)

add_executable(
    water_of_particles
    water_of_particles.cpp
    grid_collision.cpp
)

target_link_libraries(
    water_of_particles PUBLIC
    ${EXTERNAL_LIBRARIES}
)

add_executable(
    miner
    miner.cpp
)

target_link_libraries(
    miner PUBLIC
    ${EXTERNAL_LIBRARIES}
)
