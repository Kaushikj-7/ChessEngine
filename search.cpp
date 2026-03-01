#include "search.h"
#include "movegen.h"
#include "eval.h"
#include "tt.h"
#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>

// Constants
const int INF = 1000000;
const int MATE_SCORE = 30000;
const int MAX_PLY = 64;

// Search Context
Move killerMoves[MAX_PLY][2];
int historyMoves[12][64];
long long Search::nodes = 0;

// MVV-LVA tables
const int mvv_lva[6][6] = {
    {105, 205, 305, 405, 505, 605}, // Victim P, attacker P N B R Q K
    {104, 204, 304, 404, 504, 604}, // Victim N
    {103, 203, 303, 403, 503, 603}, // Victim B
    {102, 202, 302, 402, 502, 602}, // Victim R
    {101, 201, 301, 401, 501, 601}, // Victim Q
    {100, 200, 300, 400, 500, 600}  // Victim K
};

int typeOf(int p) {
    return p % 6; 
}

int scoreMove(const Board& b, const Move& m, int ply, Move ttMove) {
    if (ttMove.from == m.from && ttMove.to == m.to) return 30000; // Best move from TT
    
    if (m.captured != -1) {
        int victim = m.captured;
        int attacker = pieceAt(b, m.from);
        if (attacker == -1) return 0;
        return 10000 + mvv_lva[typeOf(victim)][typeOf(attacker)];
    }

    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        return 20000 + typeOf(m.promotion);
    }

    if (killerMoves[ply][0].from == m.from && killerMoves[ply][0].to == m.to) return 9000;
    if (killerMoves[ply][1].from == m.from && killerMoves[ply][1].to == m.to) return 8000;

    int p = pieceAt(b, m.from);
    if (p != -1) return std::min(7000, historyMoves[p][m.to]);

    return 0;
}

struct MoveScore {
    Move m;
    int score;
    bool operator>(const MoveScore& other) const { return score > other.score; }
};

int quiescence(Board& b, int alpha, int beta) {
    int standPat = Evaluator::evaluate(b);
    if (standPat >= beta) return beta;
    if (alpha < standPat) alpha = standPat;

    std::vector<Move> moves;
    MoveGenerator::generate(b, moves); 
    
    std::vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());
    for (const auto& m : moves) {
        if (m.captured != -1 || (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT)) {
             scoredMoves.push_back({m, scoreMove(b, m, 0, Move())});
        }
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<MoveScore>());

    for (const auto& ms : scoredMoves) {
        b.makeMove(ms.m);
        int score = -quiescence(b, -beta, -alpha);
        b.unmakeMove(ms.m);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// Mate score helpers
inline int scoreToTT(int score, int ply) {
    if (score >= MATE_SCORE - MAX_PLY) return score + ply;
    if (score <= -MATE_SCORE + MAX_PLY) return score - ply;
    return score;
}

inline int scoreFromTT(int score, int ply) {
    if (score >= MATE_SCORE - MAX_PLY) return score - ply;
    if (score <= -MATE_SCORE + MAX_PLY) return score + ply;
    return score;
}

int Search::alphaBeta(Board& board, int depth, int ply, int alpha, int beta) {
    nodes++;
    if (ply >= MAX_PLY - 1) return Evaluator::evaluate(board);

#ifdef USE_TT
    int ttScore;
    Move ttMove;
    if (TT.probe(board.zobristHash, depth, alpha, beta, ttScore, ttMove)) {
        // return scoreFromTT(ttScore, ply);
    }
#else
    Move ttMove;
#endif

    bool inCheck = MoveGenerator::inCheck(board, board.whiteToMove);
    if (inCheck && depth < 20) depth++; 

    if (depth <= 0) {
        return quiescence(board, alpha, beta);
    }

    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);

    if (moves.empty()) {
        if (inCheck)
            return -MATE_SCORE + ply;
        return 0; 
    }

    std::vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());
    for (const auto& m : moves) {
        scoredMoves.push_back({m, scoreMove(board, m, ply, ttMove)});
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<MoveScore>());
    
    int originalAlpha = alpha;
    Move bestMove = Move();

    bool foundPv = false;

    for (const auto& ms : scoredMoves) {
        board.makeMove(ms.m);
        
        int score;
#ifdef USE_PVS
        if (foundPv) {
            // Null window search
            score = -alphaBeta(board, depth - 1, ply + 1, -alpha - 1, -alpha);
            if (score > alpha && score < beta) {
                // Re-search with full window
                score = -alphaBeta(board, depth - 1, ply + 1, -beta, -alpha);
            }
        } else {
            score = -alphaBeta(board, depth - 1, ply + 1, -beta, -alpha);
        }
#else
        score = -alphaBeta(board, depth - 1, ply + 1, -beta, -alpha);
#endif
        board.unmakeMove(ms.m);

        if (score >= beta) {
#ifdef USE_TT
            TT.store(board.zobristHash, depth, scoreToTT(score, ply), HASH_BETA, ms.m);
#endif
            if (ms.m.captured == -1 && ms.m.promotion == -1) {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = ms.m;
                int p = pieceAt(board, ms.m.from); 
                if (p != -1) {
                    historyMoves[p][ms.m.to] += depth * depth;
                    if (historyMoves[p][ms.m.to] > 20000) historyMoves[p][ms.m.to] /= 2;
                }
            }
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            bestMove = ms.m;
            foundPv = true;
        }
    }

#ifdef USE_TT
    HashFlag flag = (alpha <= originalAlpha) ? HASH_ALPHA : HASH_EXACT;
    TT.store(board.zobristHash, depth, scoreToTT(alpha, ply), flag, bestMove);
#endif

    return alpha;
}

Move Search::findBestMove(Board& board, int depth) {
    for(int i=0; i<MAX_PLY; ++i) {
        killerMoves[i][0] = Move();
        killerMoves[i][1] = Move();
    }
    for(int i=0; i<12; ++i) for(int j=0; j<64; ++j) historyMoves[i][j] = 0;
    
    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return Move();

    Move bestMove = moves[0];
    int alpha = -INF;
    int beta = INF;

#ifdef USE_TT
    int dummy;
    Move ttMove;
    TT.probe(board.zobristHash, depth, -INF, INF, dummy, ttMove);
#else
    Move ttMove;
#endif

    std::vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());
    for (const auto& m : moves) {
        scoredMoves.push_back({m, scoreMove(board, m, 0, ttMove)});
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<MoveScore>());
    
    bool foundPv = false;

    for (const auto& ms : scoredMoves) {
        board.makeMove(ms.m);
        int score;
#ifdef USE_PVS
        if (foundPv) {
            score = -alphaBeta(board, depth - 1, 1, -alpha - 1, -alpha);
            if (score > alpha) score = -alphaBeta(board, depth - 1, 1, -beta, -alpha);
        } else {
            score = -alphaBeta(board, depth - 1, 1, -beta, -alpha);
        }
#else
        score = -alphaBeta(board, depth - 1, 1, -beta, -alpha);
#endif
        board.unmakeMove(ms.m);
        
        // std::cerr << "Move: " << ms.m.from << " to " << ms.m.to << " score: " << score << std::endl;

        if (score > alpha) {
            alpha = score;
            bestMove = ms.m;
            foundPv = true;
        }
    }
    // std::cerr << "Best: " << bestMove.from << " to " << bestMove.to << " alpha: " << alpha << std::endl;
    return bestMove;
}
