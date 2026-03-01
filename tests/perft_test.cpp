#include <iostream>
#include <vector>
#include "board.h"
#include "movegen.h"
#include "move.h"
#include "attacks.h"
#include "rays.h"

// Simple perft (node counting)
long long perft(Board& b, int depth) {
    if (depth == 0) return 1;

    std::vector<Move> moves;
    MoveGenerator::generate(b, moves);

    long long nodes = 0;
    for (const auto& move : moves) {
        Board backup = b;
        b.makeMove(move);
        // We can verify legality here if we want, but generate() should be strict now
        // If generate() generates an illegal move (leaving king in check), 
        // recursive calls would explode or logic would fail.
        // For strictly legal movegen, we trust generate() but we can assert !inCheck if we want.
        
        // Check if king is in check (illegal move made)
        // int kingSq = get_lsb(b.pieces[!b.whiteToMove ? WK : BK]); // The side that just moved
        // if (MoveGenerator::inCheck(b, !b.whiteToMove)) {
        //      std::cout << "Illegal move generated!\n";
        // }

        nodes += perft(b, depth - 1);
        b = backup;
    }
    return nodes;
}

int main() {
    initRays();
    initAttackTables(); // Ensure these are initialized

    Board b; // Start pos
    // Manually force start pos if constructor doesn't doing it correctly (assuming it does based on v2)
    
    std::cout << "Running Perft Depth 1..." << std::endl;
    long long n1 = perft(b, 1);
    std::cout << "Depth 1 Nodes: " << n1 << " (Expected 20)" << std::endl;

    std::cout << "Running Perft Depth 2..." << std::endl;
    long long n2 = perft(b, 2);
    std::cout << "Depth 2 Nodes: " << n2 << " (Expected 400)" << std::endl;

    std::cout << "Running Perft Depth 3..." << std::endl;
    long long n3 = perft(b, 3);
    std::cout << "Depth 3 Nodes: " << n3 << " (Expected 8902)" << std::endl;

    return 0;
}
