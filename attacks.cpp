#include "attacks.h"
#include "rays.h"

Bitboard KnightAttacks[64];
Bitboard KingAttacks[64];
Bitboard WhitePawnAttacks[64];
Bitboard BlackPawnAttacks[64];

static bool inited = false;

static Bitboard maskKnight(int sq) {
    Bitboard b = 0;
    int r = sq / 8, f = sq % 8;

    int dr[8] = {2,2,1,1,-1,-1,-2,-2};
    int df[8] = {1,-1,2,-2,2,-2,1,-1};

    for (int i = 0; i < 8; ++i) {
        int rr = r + dr[i], ff = f + df[i];
        if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8)
            b |= 1ULL << (rr * 8 + ff);
    }
    return b;
}

static Bitboard maskKing(int sq) {
    Bitboard b = 0;
    int r = sq / 8, f = sq % 8;

    for (int dr = -1; dr <= 1; ++dr)
        for (int df = -1; df <= 1; ++df) {
            if (dr == 0 && df == 0) continue;
            int rr = r + dr, ff = f + df;
            if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8)
                b |= 1ULL << (rr * 8 + ff);
        }
    return b;
}

void initAttackTables() {
    if (inited) return;
    
    // Leapers
    for (int sq = 0; sq < 64; ++sq) {
        KnightAttacks[sq] = maskKnight(sq);
        KingAttacks[sq] = maskKing(sq);
    }

    // Pawns
    for (int sq = 0; sq < 64; ++sq) {
        int r = sq / 8, f = sq % 8;
        Bitboard w = 0, b = 0;

        if (r < 7) {
            if (f > 0) w |= 1ULL << ((r+1)*8 + f-1);
            if (f < 7) w |= 1ULL << ((r+1)*8 + f+1);
        }
        if (r > 0) {
            if (f > 0) b |= 1ULL << ((r-1)*8 + f-1);
            if (f < 7) b |= 1ULL << ((r-1)*8 + f+1);
        }

        WhitePawnAttacks[sq] = w;
        BlackPawnAttacks[sq] = b;
    }

    inited = true;
}

static Bitboard rayAttacks(int sq, int dir, Bitboard occ) {
    Bitboard ray = Rays[dir][sq];
    Bitboard blockers = ray & occ;

    if (!blockers) return ray;

    int blockerSq;
    if (dir == N || dir == NE || dir == E || dir == NW)
        blockerSq = get_lsb(blockers);
    else
        blockerSq = get_msb(blockers);

    return ray ^ Rays[dir][blockerSq];
}

Bitboard bishopAttacks(int square, Bitboard occupancy) {
    return rayAttacks(square, NE, occupancy) |
           rayAttacks(square, NW, occupancy) |
           rayAttacks(square, SE, occupancy) |
           rayAttacks(square, SW, occupancy);
}

Bitboard rookAttacks(int square, Bitboard occupancy) {
    return rayAttacks(square, N, occupancy) |
           rayAttacks(square, S, occupancy) |
           rayAttacks(square, E, occupancy) |
           rayAttacks(square, W, occupancy);
}

Bitboard queenAttacks(int square, Bitboard occupancy) {
    return bishopAttacks(square, occupancy) | rookAttacks(square, occupancy);
}
