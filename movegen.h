#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include "move.h"
#include <vector>

class MoveGenerator {
public:
    static void generate(const Board& board, std::vector<Move>& moves);
    static bool inCheck(const Board& board, bool white);
};


#endif
