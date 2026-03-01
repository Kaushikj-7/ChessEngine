#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "bitboard.h"
#include <cstdint>

extern uint64_t PieceKeys[12][64];
extern uint64_t SideKey;
extern uint64_t CastleKeys[16];
extern uint64_t EpKeys[64];

void initZobrist();

#endif
