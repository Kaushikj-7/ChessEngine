#include "board.h"
#include "move.h"
#include <iostream>

int pieceAt(const Board& b, int sq) {
    Bitboard bit = 1ULL << sq;
    for (int p = WP; p <= BK; ++p) {
        if (b.pieces[p] & bit) return p;
    }
    return -1; // EMPTY
}

Board::Board() {
    // Empty board
    for (int i = 0; i < 12; ++i) pieces[i] = 0;
    
    // Standard Start Position (Little-endian bitboards)
    // WP rank 2 (8-15)
    pieces[WP] = 0x000000000000FF00ULL;
    // WR corners (0, 7)
    pieces[WR] = 0x0000000000000081ULL;
    // WN (1, 6)
    pieces[WN] = 0x0000000000000042ULL;
    // WB (2, 5)
    pieces[WB] = 0x0000000000000024ULL;
    // WQ (3)
    pieces[WQ] = 0x0000000000000008ULL;
    // WK (4)
    pieces[WK] = 0x0000000000000010ULL;

    // BP rank 7 (48-55)
    pieces[BP] = 0x00FF000000000000ULL;
    // BR (56, 63)
    pieces[BR] = 0x8100000000000000ULL;
    // BN (57, 62)
    pieces[BN] = 0x4200000000000000ULL;
    // BB (58, 61)
    pieces[BB] = 0x2400000000000000ULL;
    // BQ (59)
    pieces[BQ] = 0x0800000000000000ULL;
    // BK (60)
    pieces[BK] = 0x1000000000000000ULL;

    whiteToMove = true;
    enPassantSq = -1;
    castle = 0b1111; 
    history.reserve(256);
}

void Board::print() const {
    const char pieceChars[] = "PNBRQKpnbrqk";
    
    std::cout << "\n";
    for (int r = 7; r >= 0; --r) {
        std::cout << r + 1 << "  ";
        for (int f = 0; f < 8; ++f) {
            int sq = r * 8 + f;
            int piece = pieceAt(*this, sq);
            if (piece != -1) std::cout << pieceChars[piece] << " ";
            else std::cout << ". ";
        }
        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
    std::cout << "   Side: " << (whiteToMove ? "White" : "Black") << "\n";
    std::cout << "   Castle: " << ((castle & 1)?"K":"-") << ((castle & 2)?"Q":"-") 
                                 << ((castle & 4)?"k":"-") << ((castle & 8)?"q":"-") << "\n";
    std::cout << "   EP: " << enPassantSq << "\n";
}

void Board::makeMove(const Move& m) {
    history.push_back({castle, enPassantSq});

    int movedPiece = pieceAt(*this, m.from);
    
    // Updates for Castling Rights
    // If king moves, loose rights
    if (movedPiece == WK) castle &= ~0b0011;
    else if (movedPiece == BK) castle &= ~0b1100;
    
    // If rook moves or captures, loose rights
    // From Rook
    if (m.from == 0) castle &= ~0b0010; // WQ
    if (m.from == 7) castle &= ~0b0001; // WK
    if (m.from == 56) castle &= ~0b1000; // BQ
    if (m.from == 63) castle &= ~0b0100; // BK
    // To Rook (Capture) - if we capture a rook on its starting square, opponent loses casting right
    if (m.to == 0) castle &= ~0b0010;
    if (m.to == 7) castle &= ~0b0001;
    if (m.to == 56) castle &= ~0b1000;
    if (m.to == 63) castle &= ~0b0100;

    // En Passant state update
    int newEp = -1;
    if (movedPiece == WP || movedPiece == BP) {
        if (abs(m.from - m.to) == 16) {
            newEp = (m.from + m.to) / 2;
        }
    }
    enPassantSq = newEp;

    // Move Piece from Source
    pop_bit(pieces[movedPiece], m.from);

    // Handle Capture
    if (m.captured != -1) {
        if (m.promotion == ENPASSANT) {
            // Captured pawn is not at `to`
            int capSq = (whiteToMove) ? m.to - 8 : m.to + 8;
            pop_bit(pieces[m.captured], capSq);
        } else {
             pop_bit(pieces[m.captured], m.to);
        }
    }

    // Place Piece at Destination
    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        // Promotion
        set_bit(pieces[m.promotion], m.to);
    } else {
        // Normal Move (or Castle/EP King/Pawn move)
        set_bit(pieces[movedPiece], m.to);
    }

    // Special Castle Rook Move
    if (m.promotion == CASTLE) {
        if (m.to == 6) { // WK Castles KingSide e1->g1, Rook h1->f1
            pop_bit(pieces[WR], 7);
            set_bit(pieces[WR], 5);
        } else if (m.to == 2) { // WK Castles QueenSide e1->c1, Rook a1->d1
            pop_bit(pieces[WR], 0);
            set_bit(pieces[WR], 3);
        } else if (m.to == 62) { // BK Castles KingSide e8->g8, Rook h8->f8
            pop_bit(pieces[BR], 63);
            set_bit(pieces[BR], 61);
        } else if (m.to == 58) { // BK Castles QueenSide e8->c8, Rook a8->d8
            pop_bit(pieces[BR], 56);
            set_bit(pieces[BR], 59);
        }
    }

    whiteToMove = !whiteToMove;
}

void Board::unmakeMove(const Move& m) {
    whiteToMove = !whiteToMove;
    
    // Restore State
    GameState old = history.back();
    history.pop_back();
    castle = old.castle;
    enPassantSq = old.enPassantSq;

    int movedPiece = -1;
    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        movedPiece = m.promotion; // Currently on board
    } else {
        // Determine what moved based on board state? No, piece is at m.to
        // If Castle, movedPiece is King
        // If EP, movedPiece is Pawn
        movedPiece = pieceAt(*this, m.to);
    }

    // Remove from 'to'
    pop_bit(pieces[movedPiece], m.to);

    // Restore to 'from'
    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        // Verify pawn
        int pawn = whiteToMove ? WP : BP;
        set_bit(pieces[pawn], m.from);
    } else {
        set_bit(pieces[movedPiece], m.from);
    }

    // Restore Captured
    if (m.captured != -1) {
        if (m.promotion == ENPASSANT) {
            int capSq = (whiteToMove) ? m.to - 8 : m.to + 8;
             set_bit(pieces[m.captured], capSq);
        } else {
             set_bit(pieces[m.captured], m.to);
        }
    }

    // Unmake Castle Rook Move
    if (m.promotion == CASTLE) {
        if (m.to == 6) { // Undo h1->f1 => f1->h1
            pop_bit(pieces[WR], 5); set_bit(pieces[WR], 7);
        } else if (m.to == 2) { // Undo a1->d1 => d1->a1
            pop_bit(pieces[WR], 3); set_bit(pieces[WR], 0);
        } else if (m.to == 62) { 
            pop_bit(pieces[BR], 61); set_bit(pieces[BR], 63);
        } else if (m.to == 58) { 
            pop_bit(pieces[BR], 59); set_bit(pieces[BR], 56);
        }
    }
}

