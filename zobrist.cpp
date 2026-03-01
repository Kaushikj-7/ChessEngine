#include "zobrist.h"
#include <random>

uint64_t PieceKeys[12][64];
uint64_t SideKey;
uint64_t CastleKeys[16];
uint64_t EpKeys[64];

void initZobrist() {
    // Fixed seed for deterministic testing
    std::mt19937_64 rng(0x6a6b6c6d6e6f7071ULL);
    std::uniform_int_distribution<uint64_t> dist;

    for (int p = 0; p < 12; ++p) {
        for (int s = 0; s < 64; ++s) {
            PieceKeys[p][s] = dist(rng);
        }
    }

    SideKey = dist(rng);

    for (int c = 0; c < 16; ++c) {
        CastleKeys[c] = dist(rng);
    }

    for (int s = 0; s < 64; ++s) {
        EpKeys[s] = dist(rng);
    }
}
