#ifndef MOVE_H
#define MOVE_H

struct Move {
    int from;
    int to;
    char captured;
    char promotion;

    Move(int f, int t, char c = '.', char p = 0)
        : from(f), to(t), captured(c), promotion(p) {}
};

#endif
