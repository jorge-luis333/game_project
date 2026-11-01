cmake_minimum_required(VERSION 3.10)
project(Battleship CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(battleship
    src/main.cpp
    src/Board.cpp
    src/Player.cpp
    src/Console.cpp
    src/Game.cpp
)
