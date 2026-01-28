cmake_minimum_required(VERSION 3.10)

project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ChessEngine
    main.cpp
    board.cpp
    move.cpp
    movegen.cpp
)
