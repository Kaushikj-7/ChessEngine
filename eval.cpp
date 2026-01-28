#include "eval.h"

static int pieceValue(char p) {
    switch (p) {
        case 'P': return 100;
        case 'N': return 320;
        case 'B': return 330;
        case 'R': return 500;
        case 'Q': return 900;
        case 'K': return 20000;
        case 'p': return -100;
        case 'n': return -320;
        case 'b': return -330;
        case 'r': return -500;
        case 'q': return -900;
        case 'k': return -20000;
    }
    return 0;
}

int Evaluator::evaluate(const Board& board) {
    int score = 0;
    for (char p : board.squares)
        score += pieceValue(p);
    return board.whiteToMove ? score : -score;
}
