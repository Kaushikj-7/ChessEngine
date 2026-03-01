#include "eval.h"
#include "bitboard.h" 
#include <climits>

// Material weights
const int P_VAL = 100;
const int N_VAL = 320;
const int B_VAL = 330;
const int R_VAL = 500;
const int Q_VAL = 900;
const int K_VAL = 20000;

// Pawn: Encourage push, center control
const int pawn_pst[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    5,  10,  10, -20, -20,  10,  10,   5,
    5,  -5, -10,   0,   0, -10,  -5,   5,
    0,   0,   0,  20,  20,   0,   0,   0,
    5,   5,  10,  25,  25,  10,   5,   5,
   10,  10,  20,  30,  30,  20,  10,  10,
   50,  50,  50,  50,  50,  50,  50,  50,
    0,   0,   0,   0,   0,   0,   0,   0
};

// Knight: Center squares good. Edges bad.
const int knight_pst[64] = {
   -50, -40, -30, -30, -30, -30, -40, -50,
   -40, -20,   0,   5,   5,   0, -20, -40,
   -30,   5,  10,  15,  15,  10,   5, -30,
   -30,   0,  15,  20,  20,  15,   0, -30,
   -30,   5,  15,  20,  20,  15,   5, -30,
   -30,   0,  10,  15,  15,  10,   0, -30,
   -40, -20,   0,   0,   0,   0, -20, -40,
   -50, -40, -30, -30, -30, -30, -40, -50
};

// Bishop: Good diagonals, avoid corners?
const int bishop_pst[64] = {
   -20, -10, -10, -10, -10, -10, -10, -20,
   -10,   5,   0,   0,   0,   0,   5, -10,
   -10,  10,  10,  10,  10,  10,  10, -10,
   -10,   0,  10,  10,  10,  10,   0, -10,
   -10,   5,   5,  10,  10,   5,   5, -10,
   -10,   0,   5,  10,  10,   5,   0, -10,
   -10,   0,   0,   0,   0,   0,   0, -10,
   -20, -10, -10, -10, -10, -10, -10, -20
};

// Rook: 7th rank, open files? (PST just encourages centralization slightly for now)
const int rook_pst[64] = {
    0,   0,   0,   5,   5,   0,   0,   0,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
    5,  10,  10,  10,  10,  10,  10,   5,
    0,   0,   0,   0,   0,   0,   0,   0
};

// Queen: Mobility (center)
const int queen_pst[64] = {
   -20, -10, -10, -5, -5, -10, -10, -20,
   -10,   0,   0,  0,  0,   0,   0, -10,
   -10,   0,   5,  5,  5,   5,   0, -10,
    -5,   0,   5,  5,  5,   5,   0,  -5,
     0,   0,   5,  5,  5,   5,   0,  -5,
   -10,   5,   5,  5,  5,   5,   0, -10,
   -10,   0,   5,  0,  0,   0,   0, -10,
   -20, -10, -10, -5, -5, -10, -10, -20
};

// King (Middle Game): Safety in corners (g1, b1)
const int king_pst[64] = {
    20,  30,  10,   0,   0,  10,  30,  20,
    20,  20,   0,   0,   0,   0,  20,  20,
   -10, -20, -20, -20, -20, -20, -20, -10,
   -20, -30, -30, -40, -40, -30, -30, -20,
   -30, -40, -40, -50, -50, -40, -40, -30,
   -30, -40, -40, -50, -50, -40, -40, -30,
   -30, -40, -40, -50, -50, -40, -40, -30,
   -30, -40, -40, -50, -50, -40, -40, -30
};

// Helper: Mirror square index for Black
// a1 (0) -> a8 (56).  Rank r -> 7-r.
int mirrorRef(int sq) {
    int r = sq / 8;
    int f = sq % 8;
    return (7 - r) * 8 + f;
}

int evaluatePieceFuncPST(int pcType, Bitboard bb, bool white) {
    int score = 0;
    const int* table = nullptr;
    int material = 0;

    switch(pcType) {
        case 0: table = pawn_pst; material = P_VAL; break; // Pawn
        case 1: table = knight_pst; material = N_VAL; break; // Knight
        case 2: table = bishop_pst; material = B_VAL; break; // Bishop
        case 3: table = rook_pst; material = R_VAL; break; // Rook
        case 4: table = queen_pst; material = Q_VAL; break; // Queen
        case 5: table = king_pst; material = K_VAL; break; // King
    }

    while (bb) {
        int sq = get_lsb(bb);
        int pstScore = 0;
        if (white) {
            pstScore = table[sq];
        } else {
            pstScore = table[mirrorRef(sq)];
        }
        score += material + pstScore;
        bb &= bb - 1;
    }
    return score;
}


int Evaluator::evaluate(const Board& board) {
    int score = 0;

    // White pieces (0-5)
    score += evaluatePieceFuncPST(0, board.pieces[WP], true);
    score += evaluatePieceFuncPST(1, board.pieces[WN], true);
    score += evaluatePieceFuncPST(2, board.pieces[WB], true);
    score += evaluatePieceFuncPST(3, board.pieces[WR], true);
    score += evaluatePieceFuncPST(4, board.pieces[WQ], true);
    score += evaluatePieceFuncPST(5, board.pieces[WK], true);
    
    // Black pieces (0-5)
    score -= evaluatePieceFuncPST(0, board.pieces[BP], false);
    score -= evaluatePieceFuncPST(1, board.pieces[BN], false);
    score -= evaluatePieceFuncPST(2, board.pieces[BB], false);
    score -= evaluatePieceFuncPST(3, board.pieces[BR], false);
    score -= evaluatePieceFuncPST(4, board.pieces[BQ], false);
    score -= evaluatePieceFuncPST(5, board.pieces[BK], false);

    return board.whiteToMove ? score : -score;
}

