#include "rays.h"

Bitboard Rays[8][64];
static bool inited = false;

static void buildRay(int dir, int sq, int dr, int df) {
    int r = sq / 8, f = sq % 8;
    Bitboard b = 0;

    r += dr; f += df;
    while (r >= 0 && r < 8 && f >= 0 && f < 8) {
        b |= 1ULL << (r*8 + f);
        r += dr; f += df;
    }
    Rays[dir][sq] = b;
}

void initRays() {
    if (inited) return;
    for (int sq = 0; sq < 64; ++sq) {
        buildRay(N,  sq, +1,  0);
        buildRay(S,  sq, -1,  0);
        buildRay(E,  sq,  0, +1);
        buildRay(W,  sq,  0, -1);
        buildRay(NE, sq, +1, +1);
        buildRay(NW, sq, +1, -1);
        buildRay(SE, sq, -1, +1);
        buildRay(SW, sq, -1, -1);
    }
    inited = true;
}
