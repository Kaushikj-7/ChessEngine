#include "search.h"
#include "movegen.h"
#include "eval.h"
#include <vector>
#include <limits>

int Search::alphaBeta(Board& board, int depth, int alpha, int beta) {
    if (depth == 0)
        return Evaluator::evaluate(board);



    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);

    if (moves.empty())
        return Evaluator::evaluate(board);

    for (auto& m : moves) {
        board.makeMove(m);
        int score = -alphaBeta(board, depth - 1, -beta, -alpha);
        board.unmakeMove(m);

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }
    return alpha;
}

Move Search::findBestMove(Board& board, int depth) {
    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);

    if (moves.empty()) return Move(0,0);

    int bestScore = std::numeric_limits<int>::min();
    Move bestMove = moves[0];

    for (auto& m : moves) {
        board.makeMove(m);
        int score = -alphaBeta(board, depth - 1,
                                std::numeric_limits<int>::min(),
                                std::numeric_limits<int>::max());
        board.unmakeMove(m);

        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }
    }
    return bestMove;
}
