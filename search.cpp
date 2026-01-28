#include "search.h"
#include "movegen.h"
#include "eval.h"
#include <vector>
#include <limits>
#include <algorithm>

// Constants
const int INF = 1000000;
const int MATE_SCORE = 30000;
const int MAX_PLY = 64;

// Search Context (Thread local or global if single threaded)
Move killerMoves[MAX_PLY][2];
int historyMoves[12][64]; // [Piece][ToSq]

// MVV-LVA tables
const int mvv_lva[6][6] = {
    {105, 205, 305, 405, 505, 605}, // Victim P, attacker P N B R Q K
    {104, 204, 304, 404, 504, 604}, // Victim N
    {103, 203, 303, 403, 503, 603}, // Victim B
    {102, 202, 302, 402, 502, 602}, // Victim R
    {101, 201, 301, 401, 501, 601}, // Victim Q
    {100, 200, 300, 400, 500, 600}  // Victim K
};

// Helper: Get peace integer 0-5 (P-K) from Piece enum 0-11
int typeOf(int p) {
    return p % 6; 
}

int scoreMove(const Board& b, const Move& m, int ply) {
    if (m.captured != -1) {
        // En Passant capture is effectively Pawn takes Pawn
        int victim = (m.captured == -1) ? WP : m.captured; // Should be handled by m.captured logic provided
        // Correction: Move struct has 'captured' as Piece Enum.
        // For EP, captured is stored.
        
        int attacker = pieceAt(b, m.from);
        if (attacker == -1) return 0; // Should not happen

        return 10000 + mvv_lva[typeOf(victim)][typeOf(attacker)];
    }

    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        return 20000 + typeOf(m.promotion); // High priority
    }

    // Killers
    if (killerMoves[ply][0].from == m.from && killerMoves[ply][0].to == m.to) return 9000;
    if (killerMoves[ply][1].from == m.from && killerMoves[ply][1].to == m.to) return 8000;

    // History
    int p = pieceAt(b, m.from);
    if (p != -1) return historyMoves[p][m.to];

    return 0;
}

// Sorting
void pickNextMove(std::vector<Move>& moves, int currentIndex, const std::vector<int>& scores) {
    int bestIndex = currentIndex;
    int bestScore = scores[currentIndex];

    for (int i = currentIndex + 1; i < moves.size(); ++i) {
        if (scores[i] > bestScore) {
            bestScore = scores[i];
            bestIndex = i;
        }
    }

    if (bestIndex != currentIndex) {
        std::swap(moves[currentIndex], moves[bestIndex]);
        // Note: we can't easily swap scores if passed by const ref vector but we don't need to if we just need the move
        // Actually, we need to swap the score too to keep them aligned for next pick?
        // Let's implement differently: Score all at once, sort once OR Selection sort logic.
    }
}
// Actually, std::sort is fast enough for chess usually, but selection sort (pick one by one) is standard.
// Let's use a struct to keep move and score together for std::sort.
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
    MoveGenerator::generate(b, moves); // We need a 'generateCaptures' but for now generate all and filter
    
    // Sort captures? Yes, essential.
    std::vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());
    
    for (const auto& m : moves) {
        if (m.captured != -1 || m.promotion != -1) { // Only captures (and promos)
             int s = scoreMove(b, m, 0); // ply 0 or ignored for QS
             scoredMoves.push_back({m, s});
        }
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<MoveScore>());

    for (const auto& ms : scoredMoves) {
        b.makeMove(ms.m);
        // Illegal moves check? generate() is strictly legal now!
        int score = -quiescence(b, -beta, -alpha);
        b.unmakeMove(ms.m);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// Main Search
int Search::alphaBeta(Board& board, int depth, int alpha, int beta) {
    // Check extension? (Not in this phase, maybe later)
    
    if (depth <= 0) {
        return quiescence(board, alpha, beta);
        // return Evaluator::evaluate(board); // Old way
    }

    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);

    if (moves.empty()) {
        if (MoveGenerator::inCheck(board, board.whiteToMove))
            return -MATE_SCORE + (MAX_PLY - depth); // Prefer faster mate (higher ply diff)
            // Note: simple depth adjustment. Usually passed 'ply' from root.
            // For now: -30000 means mated. +depth adds value to delaying it.
        return 0; // Stalemate
    }

    // Score Moves
    std::vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());
    // We need 'ply' for killers. But this function signature doesn't have it.
    // 'depth' goes down. 'ply' goes up.
    // Let's assume passed depth is high. We can't use killers correctly without 'ply'.
    // Quick fix: Add 'ply' to arguments next time. For now, use 0.
    // Actually, I'll rewrite the signature in search.h later.
    // For now, I'll use a hack or just not use ply-based killers effectively in this function signature, 
    // OR I will overload it and call the recursive one.
    
    // Let's assume we modify the class. 
    // I'll proceed with local sorting.

    for (const auto& m : moves) {
        // Warning: 'ply' is missing. History heuristic usually ignores ply. Killers need it.
        // I will use depth as index for now (MAX_PLY - depth) -- assuming max depth is known..
        // This is messy. I should update the signature.
        int s = scoreMove(board, m, depth); 
        scoredMoves.push_back({m, s});
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<MoveScore>());
    
    int movesSearched = 0;
    for (const auto& ms : scoredMoves) {
        board.makeMove(ms.m);
        
        int score = -alphaBeta(board, depth - 1, -beta, -alpha);
        
        board.unmakeMove(ms.m);

        if (score >= beta) {
            // Beta Cutoff
            if (ms.m.captured == -1 && ms.m.promotion == -1) // Quiet move
            {
                // Update Killers
                killerMoves[depth][1] = killerMoves[depth][0];
                killerMoves[depth][0] = ms.m;
                
                // Update History
                int p = pieceAt(board, ms.m.from); 
                if (p != -1) {
                    historyMoves[p][ms.m.to] += depth * depth;
                    if (historyMoves[p][ms.m.to] > 20000) historyMoves[p][ms.m.to] /= 2; // Cap
                }
            }
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
        movesSearched++;
    }
    return alpha;
}

Move Search::findBestMove(Board& board, int depth) {
    // Reset Killers
    for(int i=0; i<MAX_PLY; ++i) {
        killerMoves[i][0] = Move(0,0);
        killerMoves[i][1] = Move(0,0);
    }
    
    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);
    
    if (moves.empty()) return Move(0,0);

    Move bestMove = moves[0];
    int alpha = -INF;
    int beta = INF;

    std::vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());
    for (const auto& m : moves) {
        int s = scoreMove(board, m, depth);
        scoredMoves.push_back({m, s});
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<MoveScore>());
    
    // Best move is the first one initially
    bestMove = scoredMoves[0].m;

    for (const auto& ms : scoredMoves) {
        board.makeMove(ms.m);
        int score = -alphaBeta(board, depth - 1, -beta, -alpha);
        board.unmakeMove(ms.m);
        
        // Simple info output
        // std::cout << "info depth " << depth << " score cp " << score << "\n";

        if (score > alpha) {
            alpha = score;
            bestMove = ms.m;
        }
    }
    return bestMove;
}
