#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"
#include <vector>
#include <cstdint>

// Piece definitions
enum Piece {
    WP, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK,
    EMPTY
};

class Move; // Forward declaration

struct GameState {
    uint8_t castle;
    int enPassantSq;
    uint64_t zobristHash;
};

class Board {
public:
    Bitboard pieces[12];
    bool whiteToMove;
    
    // State
    uint8_t castle; // bit 0: WK, 1: WQ, 2: BK, 3: BQ
    int enPassantSq; // -1 if none
    uint64_t zobristHash;
    std::vector<GameState> history;

    Board();

    // Occupancy helpers
    Bitboard whiteOcc() const {
        return pieces[WP] | pieces[WN] | pieces[WB] | pieces[WR] | pieces[WQ] | pieces[WK];
    }
    Bitboard blackOcc() const {
         return pieces[BP] | pieces[BN] | pieces[BB] | pieces[BR] | pieces[BQ] | pieces[BK];
    }
    Bitboard allOcc() const {
        return whiteOcc() | blackOcc();
    }

    void print() const;
    void loadFen(const std::string& fen);
    uint64_t computeHash() const;
    void makeMove(const Move& m);
    void unmakeMove(const Move& m);

};

// Helper to get piece at a square
int pieceAt(const Board& b, int sq);

#endif // BOARD_H
