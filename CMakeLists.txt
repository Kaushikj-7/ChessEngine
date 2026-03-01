cmake_minimum_required(VERSION 3.10)

project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ChessEngine
    main.cpp
    board.cpp
    move.cpp
    movegen.cpp
    search.cpp
    eval.cpp
    attacks.cpp
    rays.cpp
    tt.cpp
    zobrist.cpp
    uci.cpp
)

add_executable(PerftTest
    perft_test.cpp
    board.cpp
    move.cpp
    movegen.cpp
    eval.cpp
    attacks.cpp
    rays.cpp
    zobrist.cpp
)

add_executable(SymmetryTest
    symmetry_test.cpp
    board.cpp
    move.cpp
    movegen.cpp
    eval.cpp
    attacks.cpp
    rays.cpp
    zobrist.cpp
)

add_executable(MateTest
    mate_test.cpp
    board.cpp
    move.cpp
    movegen.cpp
    search.cpp
    eval.cpp
    attacks.cpp
    rays.cpp
    tt.cpp
    zobrist.cpp
)
