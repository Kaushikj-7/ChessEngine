#ifndef TT_H
#define TT_H

#include <cstdint>
#include <cstddef>
#include "move.h"

enum HashFlag {
    HASH_EXACT, HASH_ALPHA, HASH_BETA
};

struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    HashFlag flag;
    Move bestMove;
};

class TranspositionTable {
public:
    TranspositionTable(size_t sizeInMB);
    ~TranspositionTable();

    void clear();
    bool probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove);
    void store(uint64_t key, int depth, int score, HashFlag flag, Move bestMove);

private:
    TTEntry* table;
    size_t numEntries;
};

extern TranspositionTable TT;

#endif
