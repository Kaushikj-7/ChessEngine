#ifndef MOVE_H
#define MOVE_H

// Special Move Flags (stored in promotion field if not a promotion)
constexpr int CASTLE = 20;
constexpr int ENPASSANT = 21;

struct Move {
    int from;
    int to;
    int captured; // Piece enum (0-11) or -1/EMPTY
    int promotion; // Piece enum (0-11) or CASTLE/ENPASSANT or -1/NONE

    Move(int f, int t, int c = -1, int p = -1) 
        : from(f), to(t), captured(c), promotion(p) {}
};

#endif // MOVE_H
