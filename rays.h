#ifndef RAYS_H
#define RAYS_H

#include "bitboard.h"

extern Bitboard Rays[8][64];

enum RayDir {
    N, S, E, W, NE, NW, SE, SW
};

void initRays();

#endif // RAYS_H
