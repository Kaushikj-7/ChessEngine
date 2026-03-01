#include "board.h"
#include "move.h"
#include "zobrist.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

int pieceAt(const Board& b, int sq) {
    Bitboard bit = 1ULL << sq;
    for (int p = WP; p <= BK; ++p) {
        if (b.pieces[p] & bit) return p;
    }
    return -1; // EMPTY
}

Board::Board() {
    loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

uint64_t Board::computeHash() const {
    uint64_t h = 0;
    for (int p = 0; p < 12; ++p) {
        Bitboard bb = pieces[p];
        while (bb) {
            int sq = get_lsb(bb);
            h ^= PieceKeys[p][sq];
            bb &= bb - 1;
        }
    }
    if (whiteToMove) h ^= SideKey;
    h ^= CastleKeys[castle];
    if (enPassantSq != -1) h ^= EpKeys[enPassantSq];
    return h;
}

void Board::loadFen(const std::string& fen) {
    // Clear pieces
    for (int i = 0; i < 12; ++i) pieces[i] = 0;
    history.clear();

    std::istringstream ss(fen);
    std::string pos, side, castling, enp, half, full;
    ss >> pos >> side >> castling >> enp >> half >> full;

    // 1. Position
    int rank = 7, file = 0;
    for (char c : pos) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += (c - '0');
        } else {
            int sq = rank * 8 + file;
            int pc = -1;
            switch(c) {
                case 'P': pc = WP; break;
                case 'N': pc = WN; break;
                case 'B': pc = WB; break;
                case 'R': pc = WR; break;
                case 'Q': pc = WQ; break;
                case 'K': pc = WK; break;
                case 'p': pc = BP; break;
                case 'n': pc = BN; break;
                case 'b': pc = BB; break;
                case 'r': pc = BR; break;
                case 'q': pc = BQ; break;
                case 'k': pc = BK; break;
            }
            if (pc != -1) set_bit(pieces[pc], sq);
            file++;
        }
    }

    // 2. Side to move
    whiteToMove = (side == "w");

    // 3. Castling
    castle = 0;
    if (castling.find('K') != std::string::npos) castle |= 1;
    if (castling.find('Q') != std::string::npos) castle |= 2;
    if (castling.find('k') != std::string::npos) castle |= 4;
    if (castling.find('q') != std::string::npos) castle |= 8;

    // 4. En passant
    enPassantSq = -1;
    if (enp != "-") {
        int f = enp[0] - 'a';
        int r = enp[1] - '1';
        enPassantSq = r * 8 + f;
    }

    zobristHash = computeHash();
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
    std::cout << "   Hash: " << std::hex << zobristHash << std::dec << "\n";
}

void Board::makeMove(const Move& m) {
    history.push_back({castle, enPassantSq, zobristHash});

    int movedPiece = pieceAt(*this, m.from);
    
    // XOR out old castle and EP
    zobristHash ^= CastleKeys[castle];
    if (enPassantSq != -1) zobristHash ^= EpKeys[enPassantSq];

    // Updates for Castling Rights
    // If king moves, loose rights
    if (movedPiece == WK) castle &= ~0b0011;
    else if (movedPiece == BK) castle &= ~0b1100;
    
    // If rook moves or captures, loose rights
    if (m.from == 0) castle &= ~0b0010;
    if (m.from == 7) castle &= ~0b0001;
    if (m.from == 56) castle &= ~0b1000;
    if (m.from == 63) castle &= ~0b0100;
    if (m.to == 0) castle &= ~0b0010;
    if (m.to == 7) castle &= ~0b0001;
    if (m.to == 56) castle &= ~0b1000;
    if (m.to == 63) castle &= ~0b0100;

    // En Passant state update
    int newEp = -1;
    if (movedPiece == WP || movedPiece == BP) {
        if (std::abs(m.from - m.to) == 16) {
            newEp = (m.from + m.to) / 2;
        }
    }
    enPassantSq = newEp;

    // XOR in new castle and EP
    zobristHash ^= CastleKeys[castle];
    if (enPassantSq != -1) zobristHash ^= EpKeys[enPassantSq];

    // XOR out moved piece at from
    zobristHash ^= PieceKeys[movedPiece][m.from];
    pop_bit(pieces[movedPiece], m.from);

    // Handle Capture
    if (m.captured != -1) {
        if (m.promotion == ENPASSANT) {
            int capSq = (whiteToMove) ? m.to - 8 : m.to + 8;
            zobristHash ^= PieceKeys[m.captured][capSq];
            pop_bit(pieces[m.captured], capSq);
        } else {
            zobristHash ^= PieceKeys[m.captured][m.to];
            pop_bit(pieces[m.captured], m.to);
        }
    }

    // Place Piece at Destination
    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        zobristHash ^= PieceKeys[m.promotion][m.to];
        set_bit(pieces[m.promotion], m.to);
    } else {
        zobristHash ^= PieceKeys[movedPiece][m.to];
        set_bit(pieces[movedPiece], m.to);
    }

    // Special Castle Rook Move
    if (m.promotion == CASTLE) {
        if (m.to == 6) { // h1->f1
            zobristHash ^= PieceKeys[WR][7]; zobristHash ^= PieceKeys[WR][5];
            pop_bit(pieces[WR], 7); set_bit(pieces[WR], 5);
        } else if (m.to == 2) { // a1->d1
            zobristHash ^= PieceKeys[WR][0]; zobristHash ^= PieceKeys[WR][3];
            pop_bit(pieces[WR], 0); set_bit(pieces[WR], 3);
        } else if (m.to == 62) { // h8->f8
            zobristHash ^= PieceKeys[BR][63]; zobristHash ^= PieceKeys[BR][61];
            pop_bit(pieces[BR], 63); set_bit(pieces[BR], 61);
        } else if (m.to == 58) { // a8->d8
            zobristHash ^= PieceKeys[BR][56]; zobristHash ^= PieceKeys[BR][59];
            pop_bit(pieces[BR], 56); set_bit(pieces[BR], 59);
        }
    }

    whiteToMove = !whiteToMove;
    zobristHash ^= SideKey;
}

void Board::unmakeMove(const Move& m) {
    whiteToMove = !whiteToMove;
    
    // Restore State
    GameState old = history.back();
    history.pop_back();
    castle = old.castle;
    enPassantSq = old.enPassantSq;
    zobristHash = old.zobristHash;

    int movedPiece = -1;
    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        movedPiece = m.promotion;
    } else {
        movedPiece = pieceAt(*this, m.to);
    }

    pop_bit(pieces[movedPiece], m.to);
    if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
        int pawn = whiteToMove ? WP : BP;
        set_bit(pieces[pawn], m.from);
    } else {
        set_bit(pieces[movedPiece], m.from);
    }

    if (m.captured != -1) {
        if (m.promotion == ENPASSANT) {
            int capSq = (whiteToMove) ? m.to - 8 : m.to + 8;
             set_bit(pieces[m.captured], capSq);
        } else {
             set_bit(pieces[m.captured], m.to);
        }
    }

    if (m.promotion == CASTLE) {
        if (m.to == 6) { pop_bit(pieces[WR], 5); set_bit(pieces[WR], 7); }
        else if (m.to == 2) { pop_bit(pieces[WR], 3); set_bit(pieces[WR], 0); }
        else if (m.to == 62) { pop_bit(pieces[BR], 61); set_bit(pieces[BR], 63); }
        else if (m.to == 58) { pop_bit(pieces[BR], 59); set_bit(pieces[BR], 56); }
    }
}

