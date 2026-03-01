#include "tt.h"
#include <cstring>
#include <iostream>

TranspositionTable TT(64); // Default 64MB

TranspositionTable::TranspositionTable(size_t sizeInMB) {
    numEntries = (sizeInMB * 1024 * 1024) / sizeof(TTEntry);
    table = new TTEntry[numEntries];
    clear();
}

TranspositionTable::~TranspositionTable() {
    delete[] table;
}

void TranspositionTable::clear() {
    for (size_t i = 0; i < numEntries; ++i) {
        table[i].key = 0;
        table[i].depth = -1;
        table[i].score = 0;
        table[i].flag = HASH_ALPHA;
        table[i].bestMove = Move();
    }
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove) {
    TTEntry& entry = table[key % numEntries];
    if (entry.key == key) {
        bestMove = entry.bestMove;
        if (entry.depth >= depth) {
            if (entry.flag == HASH_EXACT) {
                score = entry.score;
                return true;
            }
            if (entry.flag == HASH_ALPHA && entry.score <= alpha) {
                score = entry.score;
                return true;
            }
            if (entry.flag == HASH_BETA && entry.score >= beta) {
                score = entry.score;
                return true;
            }
        }
    }
    return false;
}

void TranspositionTable::store(uint64_t key, int depth, int score, HashFlag flag, Move bestMove) {
    TTEntry& entry = table[key % numEntries];
    
    // Depth-preferred replacement
    bool samePos = (entry.key == key);
    if (entry.key == 0 || depth >= entry.depth || !samePos) {
        entry.key = key;
        entry.depth = depth;
        entry.score = score;
        entry.flag = flag;
        if (bestMove.from != -1) {
            entry.bestMove = bestMove;
        }
    }
}
