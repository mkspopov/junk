add_library(
    particles
    particles.cpp particles.h
)

find_package(SFML REQUIRED COMPONENTS audio graphics network system window)

target_link_libraries(
    particles PUBLIC
    sfml-audio
    sfml-graphics
    sfml-network
    sfml-system
    sfml-window
)

add_executable(
    main
    main.cpp
)

target_link_libraries(
    main
    particles
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
