#include "board.h"
#include "search.h"
#include <iostream>

int main() {
    Board board;

    for (int turn = 0; turn < 40; ++turn) {
        board.print();
        Move best = Search::findBestMove(board, 2);
        board.makeMove(best);
    }
    board.print();
    return 0;
}
