#include <iostream>
#include <vector>
#include <cassert>
#include "board.h"
#include "move.h"
#include "eval.h"
#include "attacks.h"
#include "rays.h"

// Helper to mirror square index vertically
int mirror(int sq) {
    return (7 - (sq / 8)) * 8 + (sq % 8);
}

Board flipBoard(const Board& b) {
    Board flipped;
    // Clear pieces
    for (int i = 0; i < 12; ++i) flipped.pieces[i] = 0;

    // Flip pieces and colors
    for (int p = 0; p < 6; ++p) {
        // White pieces in original (p) -> Black pieces in flipped (p+6)
        Bitboard whiteBb = b.pieces[p];
        while (whiteBb) {
            int sq = get_lsb(whiteBb);
            set_bit(flipped.pieces[p + 6], mirror(sq));
            whiteBb &= whiteBb - 1;
        }

        // Black pieces in original (p+6) -> White pieces in flipped (p)
        Bitboard blackBb = b.pieces[p + 6];
        while (blackBb) {
            int sq = get_lsb(blackBb);
            set_bit(flipped.pieces[p], mirror(sq));
            blackBb &= blackBb - 1;
        }
    }

    flipped.whiteToMove = !b.whiteToMove;
    // Castle rights also need flipping (WK/WQ <-> BK/BQ)
    flipped.castle = 0;
    if (b.castle & 1) flipped.castle |= 4; // WK -> BK
    if (b.castle & 2) flipped.castle |= 8; // WQ -> BQ
    if (b.castle & 4) flipped.castle |= 1; // BK -> WK
    if (b.castle & 8) flipped.castle |= 2; // BQ -> WQ

    if (b.enPassantSq != -1) {
        flipped.enPassantSq = mirror(b.enPassantSq);
    } else {
        flipped.enPassantSq = -1;
    }

    return flipped;
}

int main() {
    initRays();
    initAttackTables();

    Board b; // Start pos
    int eval1 = Evaluator::evaluate(b);
    
    Board fb = flipBoard(b);
    int eval2 = Evaluator::evaluate(fb);

    std::cout << "Start Pos: " << eval1 << " == " << eval2 << std::endl;
    assert(eval1 == eval2);

    // Test with move e2e4 (White)
    // e2 = 12, e4 = 28
    // WP rank 2 (8-15) -> 8+4=12. rank 4 (24-31) -> 24+4=28.
    Move m_white(12, 28, -1, -1);
    b.makeMove(m_white);
    eval1 = Evaluator::evaluate(b);

    // Mirrored move for Black in the flipped board: e7e5
    // e7 = 52, e5 = 36 (r6*8+4 = 52, r4*8+4 = 36)
    // Mirror of 12 (r1,f4) is 52 (r6,f4). Mirror of 28 (r3,f4) is 36 (r4,f4).
    // In fb, whiteToMove is false (Black to move).
    Move m_black(mirror(12), mirror(28), -1, -1);
    fb.makeMove(m_black);
    eval2 = Evaluator::evaluate(fb);

    std::cout << "After e2e4 (W) / e7e5 (B): " << eval1 << " == " << eval2 << std::endl;
    
    if (eval1 == eval2) {
        std::cout << "Symmetry Test PASSED for Start Position and moves." << std::endl;
    } else {
        std::cout << "Symmetry Test FAILED after moves!" << std::endl;
        return 1;
    }

    // Test with a non-symmetric position?
    // Let's move e2e4
    // Move m(12, 28); // e2 is 12, e4 is 28 (r1*8+4 = 12, r3*8+4 = 28)
    // Actually Piece enum: WP is 0. 
    // b.makeMove(Move(12, 28));
    
    // For now, let's just do start pos. 
    // Usually start pos eval is 0 anyway if perfectly symmetric.
    
    return 0;
}
