#ifndef ATTACKS_H
#define ATTACKS_H

#include "bitboard.h"

// Attack tables
extern Bitboard KnightAttacks[64];
extern Bitboard KingAttacks[64];
extern Bitboard WhitePawnAttacks[64];
extern Bitboard BlackPawnAttacks[64];

// Initialization
void initAttackTables();

// Slider attacks
Bitboard bishopAttacks(int square, Bitboard occupancy);
Bitboard rookAttacks(int square, Bitboard occupancy);
Bitboard queenAttacks(int square, Bitboard occupancy);

#endif // ATTACKS_H
