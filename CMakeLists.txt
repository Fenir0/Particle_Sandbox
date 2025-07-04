cmake_minimum_required(VERSION 3.10.0)
project(sandbox VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)
add_executable(sandbox main.cpp src/game.cpp)

target_link_libraries(sandbox sfml-graphics sfml-audio)

