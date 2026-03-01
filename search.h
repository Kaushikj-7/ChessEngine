#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "move.h"

class Search {
public:
    static int alphaBeta(Board& board, int depth, int ply, int alpha, int beta);
    static Move findBestMove(Board& board, int depth);
    static long long nodes;
};

#endif

