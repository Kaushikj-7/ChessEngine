#include <iostream>
#include <vector>
#include <cassert>
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "zobrist.h"
#include "attacks.h"
#include "rays.h"

int main() {
    initRays();
    initAttackTables();
    initZobrist();

    Board b;
    std::cout << "Start Hash: " << std::hex << b.zobristHash << std::endl;
    assert(b.zobristHash == b.computeHash());

    // Make some moves and check consistency
    std::vector<Move> moves;
    MoveGenerator::generate(b, moves);

    for (int i = 0; i < std::min((int)moves.size(), 10); ++i) {
        Move m = moves[i];
        b.makeMove(m);
        uint64_t incremental = b.zobristHash;
        uint64_t full = b.computeHash();
        
        if (incremental != full) {
            std::cout << "Hash mismatch after move " << i << "!" << std::endl;
            std::cout << "Incremental: " << std::hex << incremental << std::endl;
            std::cout << "Full: " << std::hex << full << std::endl;
            return 1;
        }
        b.unmakeMove(m);
        if (b.zobristHash != b.computeHash()) {
            std::cout << "Hash mismatch after unmake move " << i << "!" << std::endl;
            return 1;
        }
    }

    std::cout << "Hash consistency test PASSED." << std::endl;
    return 0;
}
