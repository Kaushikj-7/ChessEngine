#include "board.h"
#include <iostream>
uint64_t Board::hash() const {
    uint64_t h = 1469598103934665603ULL;
    for (char p : squares) {
        h ^= (uint64_t)p;
        h *= 1099511628211ULL;
    }
    h ^= whiteToMove;
    return h;
}

Board::Board() {
    const char init[64] = {
        'r','n','b','q','k','b','n','r',
        'p','p','p','p','p','p','p','p',
        '.','.','.','.','.','.','.','.',
        '.','.','.','.','.','.','.','.',
        '.','.','.','.','.','.','.','.',
        '.','.','.','.','.','.','.','.',
        'P','P','P','P','P','P','P','P',
        'R','N','B','Q','K','B','N','R'
    };

    for (int i = 0; i < 64; ++i) squares[i] = init[i];
    whiteToMove = true;
}

void Board::print() const {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f)
            std::cout << squares[r * 8 + f] << " ";
        std::cout << "\n";
    }
    std::cout << "\n";
}

void Board::makeMove(const Move& m) {
    squares[m.to] = squares[m.from];
    squares[m.from] = '.';
    whiteToMove = !whiteToMove;
}

void Board::unmakeMove(const Move& m) {
    squares[m.from] = squares[m.to];
    squares[m.to] = m.captured;
    whiteToMove = !whiteToMove;
}
