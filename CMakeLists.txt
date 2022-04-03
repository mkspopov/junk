cmake_minimum_required(VERSION 3.18)
project(particles)

set(CMAKE_CXX_STANDARD 20)

if (DEFINED $ENV{ASAN})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif()

add_subdirectory(src)
