cmake_minimum_required(VERSION 3.10.0)
project(sandbox VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

set(HEADERS include/game.hpp include/particle.hpp include/update.hpp)
set(SOURCE  main.cpp src/particle.cpp   src/game.cpp src/update.cpp)


add_executable(sandbox ${HEADERS} ${SOURCE})

target_link_libraries(sandbox sfml-graphics sfml-audio)

