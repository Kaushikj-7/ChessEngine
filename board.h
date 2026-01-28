#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <cstdint>
#include "move.h"

class Board {
public:
    std::array<char, 64> squares;
    bool whiteToMove;
    uint64_t hash() const;


    Board();
    void print() const;

    void makeMove(const Move& m);
    void unmakeMove(const Move& m);
};

#endif
