#include "movegen.h"
#include "attacks.h"
#include "rays.h"
#include <iostream> 

// Helper for CLZ/CTZ compatibility across compilers if needed
// For g++, get_lsb is fine.

// Forward declarations
Bitboard attackersTo(int sq, const Board& b, bool byWhite, Bitboard occ);

// Step 1: Detect Attacks
Bitboard attackersTo(int sq, const Board& b, bool byWhite, Bitboard occ) {
    Bitboard atk = 0;

    // Pawns
    Bitboard pawnAtk = byWhite ? WhitePawnAttacks[sq] : BlackPawnAttacks[sq];
    atk |= pawnAtk & (byWhite ? b.pieces[WP] : b.pieces[BP]);

    // Knights
    if (KnightAttacks[sq] & (byWhite ? b.pieces[WN] : b.pieces[BN]))
        atk |= KnightAttacks[sq] & (byWhite ? b.pieces[WN] : b.pieces[BN]);

    // Bishop/Queen
    Bitboard bq = (byWhite ? (b.pieces[WB]|b.pieces[WQ]) : (b.pieces[BB]|b.pieces[BQ]));
    if (bq) {
        Bitboard batk = bishopAttacks(sq, occ);
        if (batk & bq) atk |= (batk & bq);
    }

    // Rook/Queen
    Bitboard rq = (byWhite ? (b.pieces[WR]|b.pieces[WQ]) : (b.pieces[BR]|b.pieces[BQ]));
    if (rq) {
        Bitboard ratk = rookAttacks(sq, occ);
        if (ratk & rq) atk |= (ratk & rq);
    }
    
    // King (needed for symmetry, king cannot attack king but blocks squares)
    Bitboard k = byWhite ? b.pieces[WK] : b.pieces[BK];
    if (KingAttacks[sq] & k) atk |= k;

    return atk;
}
// Overload for default occupancy
Bitboard attackersTo(int sq, const Board& b, bool byWhite) {
    return attackersTo(sq, b, byWhite, b.allOcc());
}

// Step 2: Detect Pins
void detectPins(const Board& b, bool white, Bitboard& pinned, Bitboard& pinRay) {
    pinned = 0;
    pinRay = 0;

    int p = white ? WK : BK;
    if (b.pieces[p] == 0) return;
    int ksq = get_lsb(b.pieces[p]);
    
    Bitboard occ = b.allOcc();

    static int dirs[8] = {N,S,E,W,NE,NW,SE,SW};

    for (int d = 0; d < 8; ++d) {
        Bitboard ray = Rays[dirs[d]][ksq];
        Bitboard blockers = ray & occ;
        if (!blockers) continue;

        int first;
        if (dirs[d] == N || dirs[d] == NE || dirs[d] == E || dirs[d] == NW)
            first = get_lsb(blockers);
        else 
            first = get_msb(blockers);

        Bitboard behind = Rays[dirs[d]][first] & occ;
        if (!behind) continue; 

        int second;
        if (dirs[d] == N || dirs[d] == NE || dirs[d] == E || dirs[d] == NW)
             second = get_lsb(behind);
        else second = get_msb(behind);

        int dir = dirs[d];
        bool diag = (dir == NE || dir == NW || dir == SE || dir == SW);
        bool ortho = (dir == N || dir == S || dir == E || dir == W);

        Bitboard enemyRQ = white ? (b.pieces[BR]|b.pieces[BQ]) : (b.pieces[WR]|b.pieces[WQ]);
        Bitboard enemyBQ = white ? (b.pieces[BB]|b.pieces[BQ]) : (b.pieces[WB]|b.pieces[WQ]);

        Bitboard secondBit = (1ULL << second);
        bool pinner = false;

        if (ortho && (enemyRQ & secondBit)) pinner = true;
        if (diag && (enemyBQ & secondBit)) pinner = true;

        if (pinner) {
            Bitboard own = white ? b.whiteOcc() : b.blackOcc();
            if (own & (1ULL << first)) {
                pinned |= (1ULL << first);
                pinRay |= Rays[dirs[d]][ksq]; 
            }
        }
    }
}

// Helpers for check evasion
static int getDir(int from, int to) {
    if (Rays[N][from] & (1ULL<<to)) return N;
    if (Rays[S][from] & (1ULL<<to)) return S;
    if (Rays[E][from] & (1ULL<<to)) return E;
    if (Rays[W][from] & (1ULL<<to)) return W;
    if (Rays[NE][from] & (1ULL<<to)) return NE;
    if (Rays[NW][from] & (1ULL<<to)) return NW;
    if (Rays[SE][from] & (1ULL<<to)) return SE;
    if (Rays[SW][from] & (1ULL<<to)) return SW;
    return -1;
}

static void genRooks(const Board& b, std::vector<Move>& moves, Bitboard pinned, Bitboard pinRay, Bitboard validMask) {
    int p = b.whiteToMove ? WR : BR;
    Bitboard rooks = b.pieces[p];
    Bitboard own = b.whiteToMove ? b.whiteOcc() : b.blackOcc();
    Bitboard opp = b.whiteToMove ? b.blackOcc() : b.whiteOcc();
    Bitboard occ = b.allOcc();

    while (rooks) {
        int from = get_lsb(rooks);
        rooks &= rooks - 1;

        Bitboard mask = validMask;
        if ((1ULL << from) & pinned) mask &= pinRay;

        Bitboard targets = rookAttacks(from, occ) & ~own & mask;

        while (targets) {
            int to = get_lsb(targets);
            int cap = (opp & (1ULL << to)) ? pieceAt(b, to) : -1;
            moves.emplace_back(from, to, cap);
            targets &= targets - 1;
        }
    }
}

static void genBishops(const Board& b, std::vector<Move>& moves, Bitboard pinned, Bitboard pinRay, Bitboard validMask) {
    int p = b.whiteToMove ? WB : BB;
    Bitboard bishops = b.pieces[p];
    Bitboard own = b.whiteToMove ? b.whiteOcc() : b.blackOcc();
    Bitboard opp = b.whiteToMove ? b.blackOcc() : b.whiteOcc();
    Bitboard occ = b.allOcc();

    while (bishops) {
        int from = get_lsb(bishops);
        bishops &= bishops - 1;

        Bitboard mask = validMask;
        if ((1ULL << from) & pinned) mask &= pinRay;

        Bitboard targets = bishopAttacks(from, occ) & ~own & mask;

        while (targets) {
            int to = get_lsb(targets);
            int cap = (opp & (1ULL << to)) ? pieceAt(b, to) : -1;
            moves.emplace_back(from, to, cap);
            targets &= targets - 1;
        }
    }
}

static void genQueens(const Board& b, std::vector<Move>& moves, Bitboard pinned, Bitboard pinRay, Bitboard validMask) {
    int p = b.whiteToMove ? WQ : BQ;
    Bitboard queens = b.pieces[p];
    Bitboard own = b.whiteToMove ? b.whiteOcc() : b.blackOcc();
    Bitboard opp = b.whiteToMove ? b.blackOcc() : b.whiteOcc();
    Bitboard occ = b.allOcc();

    while (queens) {
        int from = get_lsb(queens);
        queens &= queens - 1;

        Bitboard mask = validMask;
        if ((1ULL << from) & pinned) mask &= pinRay;

        Bitboard targets = queenAttacks(from, occ) & ~own & mask;

        while (targets) {
            int to = get_lsb(targets);
            int cap = (opp & (1ULL << to)) ? pieceAt(b, to) : -1;
            moves.emplace_back(from, to, cap);
            targets &= targets - 1;
        }
    }
}

static void genKnights(const Board& b, std::vector<Move>& moves, Bitboard pinned, Bitboard validMask) {
    Bitboard knights = b.whiteToMove ? b.pieces[WN] : b.pieces[BN];
    Bitboard own = b.whiteToMove ? b.whiteOcc() : b.blackOcc();
    Bitboard opp = b.whiteToMove ? b.blackOcc() : b.whiteOcc();

    // Pinned knights cannot move
    knights &= ~pinned; 

    while (knights) {
        int from = get_lsb(knights);
        Bitboard targets = KnightAttacks[from] & ~own & validMask;

        while (targets) {
            int to = get_lsb(targets);
            int cap = (opp & (1ULL << to)) ? pieceAt(b, to) : -1;
            moves.emplace_back(from, to, cap);
            targets &= targets - 1;
        }
        knights &= knights - 1;
    }
}

static void genKing(const Board& b, std::vector<Move>& moves) {
    int p = b.whiteToMove ? WK : BK;
    Bitboard king = b.pieces[p];
    if (!king) return; 

    int from = get_lsb(king);

    Bitboard own = b.whiteToMove ? b.whiteOcc() : b.blackOcc();
    Bitboard opp = b.whiteToMove ? b.blackOcc() : b.whiteOcc();
    // Step 4: King Move Safety
    // We must check attacks on target squares WITHOUT the king on the board
    // to detect x-rays behind the king.
    Bitboard occWithoutKing = b.allOcc() ^ king;

    Bitboard targets = KingAttacks[from] & ~own;

    while (targets) {
        int to = get_lsb(targets);
        if (!attackersTo(to, b, !b.whiteToMove, occWithoutKing)) {
             int cap = (opp & (1ULL << to)) ? pieceAt(b, to) : -1;
             moves.emplace_back(from, to, cap);
        }
        targets &= targets - 1;
    }

    // Castling
    // Prereq: King not in check
    if (attackersTo(from, b, !b.whiteToMove, occWithoutKing)) return;

    Bitboard all = b.allOcc();

    if (b.whiteToMove) {
        // White Kingside (Bit 0) - e1->g1 (4->6). Spans f1(5), g1(6).
        if ((b.castle & 1) && !(all & ((1ULL<<5)|(1ULL<<6)))) {
             if (!attackersTo(5, b, false, all) && !attackersTo(6, b, false, all)) {
                  moves.emplace_back(4, 6, -1, CASTLE);
             }
        }
        // White Queenside (Bit 1) - e1->c1 (4->2). Spans d1(3), c1(2), b1(1).
        // Note: b1 must be empty, but does not need to be safe.
        // d1, c1 must be empty AND safe.
        // Wait, standard rules:
        // "The squares between the king and the rook must be vacant." (d1, c1, b1)
        // "The king does not pass through ... an attacked square." (d1, c1)
        if ((b.castle & 2) && !(all & ((1ULL<<1)|(1ULL<<2)|(1ULL<<3)))) {
             if (!attackersTo(3, b, false, all) && !attackersTo(2, b, false, all)) {
                  moves.emplace_back(4, 2, -1, CASTLE);
             }
        }
    } else {
        // Black Kingside (Bit 2) - e8->g8 (60->62). Spans f8(61), g8(62)
        if ((b.castle & 4) && !(all & ((1ULL<<61)|(1ULL<<62)))) {
             if (!attackersTo(61, b, true, all) && !attackersTo(62, b, true, all)) {
                  moves.emplace_back(60, 62, -1, CASTLE);
             }
        }
        // Black Queenside (Bit 3) - e8->c8 (60->58). Spans d8(59), c8(58), b8(57)
        if ((b.castle & 8) && !(all & ((1ULL<<57)|(1ULL<<58)|(1ULL<<59)))) {
             if (!attackersTo(59, b, true, all) && !attackersTo(58, b, true, all)) {
                  moves.emplace_back(60, 58, -1, CASTLE);
             }
        }
    }
}

static void addPawnMove(const Board& b, std::vector<Move>& moves, int from, int to, int cap) {
     if (to >= 56 || to <= 7) {
        moves.emplace_back(from, to, cap, b.whiteToMove ? WQ : BQ);
        moves.emplace_back(from, to, cap, b.whiteToMove ? WR : BR);
        moves.emplace_back(from, to, cap, b.whiteToMove ? WB : BB);
        moves.emplace_back(from, to, cap, b.whiteToMove ? WN : BN);
    } else {
        moves.emplace_back(from, to, cap);
    }
}

static void genPawns(const Board& b, std::vector<Move>& moves, Bitboard pinned, Bitboard pinRay, Bitboard validMask) {
    Bitboard pawns = b.whiteToMove ? b.pieces[WP] : b.pieces[BP];
    Bitboard empty = ~b.allOcc();
    Bitboard enemy = b.whiteToMove ? b.blackOcc() : b.whiteOcc();

    if (b.whiteToMove) {
        // Single Push
        Bitboard singlePush = (pawns << 8) & empty;
        Bitboard validPush = singlePush & validMask;

        while (validPush) {
            int to = get_lsb(validPush);
            int from = to - 8;
            bool isPinned = (pinned & (1ULL << from));
            if (!isPinned || (pinRay & (1ULL << to))) {
                 addPawnMove(b, moves, from, to, -1);
            }
            validPush &= validPush - 1;
        }
        
        // Double Push (Rank 2 is 8-15. Rank 3 is 16-23. Rank 4 is 24-31)
        Bitboard doublePush = ((singlePush & (0xFFULL << 16)) << 8) & empty & validMask;
        while (doublePush) {
             int to = get_lsb(doublePush);
             int from = to - 16;
             bool isPinned = (pinned & (1ULL << from));
             if (!isPinned || (pinRay & (1ULL << to)))
                 moves.emplace_back(from, to, -1);
             doublePush &= doublePush - 1;
        }

        // Captures
        const Bitboard notA = 0xfefefefefefefefeULL;
        const Bitboard notH = 0x7f7f7f7f7f7f7f7fULL;

        Bitboard leftCap = ((pawns & notA) << 7) & enemy & validMask;
        while(leftCap) {
             int to = get_lsb(leftCap);
             int from = to - 7;
             bool isPinned = (pinned & (1ULL << from));
             if (!isPinned || (pinRay & (1ULL << to)))
                 addPawnMove(b, moves, from, to, pieceAt(b, to));
             leftCap &= leftCap - 1;
        }

        Bitboard rightCap = ((pawns & notH) << 9) & enemy & validMask;
        while(rightCap) {
             int to = get_lsb(rightCap);
             int from = to - 9;
             bool isPinned = (pinned & (1ULL << from));
             if (!isPinned || (pinRay & (1ULL << to)))
                addPawnMove(b, moves, from, to, pieceAt(b, to));
             rightCap &= rightCap - 1;
        }

        // En Passant
        if (b.enPassantSq != -1) {
             int ep = b.enPassantSq; 
             Bitboard epAttacks = BlackPawnAttacks[ep] & pawns;
             while (epAttacks) {
                  int from = get_lsb(epAttacks);
                  int capSq = ep - 8;
                  
                  Bitboard newOcc = b.allOcc();
                  newOcc ^= (1ULL << from);
                  newOcc ^= (1ULL << capSq); 
                  newOcc |= (1ULL << ep);    

                  int ksq = get_lsb(b.pieces[WK]);
                  Bitboard rooksQueens = b.pieces[BR] | b.pieces[BQ];
                  Bitboard bishopsQueens = b.pieces[BB] | b.pieces[BQ];
                  
                  bool safe = true;
                  if (rookAttacks(ksq, newOcc) & rooksQueens) safe = false;
                  if (bishopAttacks(ksq, newOcc) & bishopsQueens) safe = false;

                  if (safe) {
                       moves.emplace_back(from, ep, pieceAt(b, capSq), ENPASSANT);
                  }
                  
                  epAttacks &= epAttacks - 1;
             }
        }

    } else {
        // Black Single Push ( -8 )
        Bitboard singlePush = (pawns >> 8) & empty;
        Bitboard validPush = singlePush & validMask;
        while (validPush) {
            int to = get_lsb(validPush);
            int from = to + 8;
            bool isPinned = (pinned & (1ULL << from));
            if (!isPinned || (pinRay & (1ULL << to)))
                addPawnMove(b, moves, from, to, -1);
            validPush &= validPush - 1;
        }

        // Black Double Push (Rank 7 is 48-55. Rank 6 is 40-47. Rank 5 is 32-39)
        Bitboard doublePush = ((singlePush & (0xFFULL << 40)) >> 8) & empty & validMask;
        while (doublePush) {
             int to = get_lsb(doublePush);
             int from = to + 16;
             bool isPinned = (pinned & (1ULL << from));
             if (!isPinned || (pinRay & (1ULL << to)))
                 moves.emplace_back(from, to, -1);
             doublePush &= doublePush - 1;
        }

        const Bitboard notA = 0xfefefefefefefefeULL;
        const Bitboard notH = 0x7f7f7f7f7f7f7f7fULL;

        Bitboard leftCap = ((pawns & notA) >> 9) & enemy & validMask;
        while(leftCap) {
             int to = get_lsb(leftCap);
             int from = to + 9;
             bool isPinned = (pinned & (1ULL << from));
             if (!isPinned || (pinRay & (1ULL << to)))
                addPawnMove(b, moves, from, to, pieceAt(b, to));
             leftCap &= leftCap - 1;
        }

        Bitboard rightCap = ((pawns & notH) >> 7) & enemy & validMask; 
        while(rightCap) {
             int to = get_lsb(rightCap);
             int from = to + 7;
             bool isPinned = (pinned & (1ULL << from));
             if (!isPinned || (pinRay & (1ULL << to)))
                addPawnMove(b, moves, from, to, pieceAt(b, to));
             rightCap &= rightCap - 1;
        }

        // En Passant
        if (b.enPassantSq != -1) {
             int ep = b.enPassantSq; 
             Bitboard epAttacks = WhitePawnAttacks[ep] & pawns;
             while (epAttacks) {
                  int from = get_lsb(epAttacks);
                  int capSq = ep + 8;
                  
                  Bitboard newOcc = b.allOcc();
                  newOcc ^= (1ULL << from);
                  newOcc ^= (1ULL << capSq);
                  newOcc |= (1ULL << ep);

                  int ksq = get_lsb(b.pieces[BK]);
                  Bitboard rooksQueens = b.pieces[WR] | b.pieces[WQ];
                  Bitboard bishopsQueens = b.pieces[WB] | b.pieces[WQ];
                  
                  bool safe = true;
                  if (rookAttacks(ksq, newOcc) & rooksQueens) safe = false;
                  if (bishopAttacks(ksq, newOcc) & bishopsQueens) safe = false;
                  
                  if (safe) {
                       moves.emplace_back(from, ep, pieceAt(b, capSq), ENPASSANT);
                  }
                  
                  epAttacks &= epAttacks - 1;
             }
        }
    }
}

void MoveGenerator::generate(const Board& b, std::vector<Move>& moves) {
    moves.clear();
    
    // King location
    int p = b.whiteToMove ? WK : BK;
    int ksq = get_lsb(b.pieces[p]);

    // Attackers to King (Checkers)
    Bitboard checkers = attackersTo(ksq, b, !b.whiteToMove, b.allOcc());

    // Pin Detection
    Bitboard pinned = 0;
    Bitboard pinRay = 0;
    detectPins(b, b.whiteToMove, pinned, pinRay);

    // Valid Mask (Step 5)
    Bitboard validMask = ~0ULL;
    int numCheckers = count_bits(checkers);

    if (numCheckers > 1) {
        // Double Check: Only King moves allowed
        validMask = 0;
    } else if (numCheckers == 1) {
        // Single Check: Capture or Block
        int cSq = get_lsb(checkers);
        Bitboard checkerBit = (1ULL << cSq);
        
        // Try to find ray
        int dir = getDir(ksq, cSq);
        if (dir != -1) {
            // Slider check - can block
             // Ray from ksq(excl) to cSq(incl)? Use XOR logic
            Bitboard rayMask = (Rays[dir][ksq] ^ Rays[dir][cSq]) | checkerBit; 
            validMask = rayMask;
        } else {
            // Knight/Pawn check - must capture
            validMask = checkerBit; 
        }
    }

    // Generate Moves
    genKing(b, moves); // King logic filters its own safety
    
    // If double check, only king moves allowed.
    if (numCheckers > 1) return;

    // Normal generation with masks
    genKnights(b, moves, pinned, validMask);
    genPawns(b, moves, pinned, pinRay, validMask);
    genRooks(b, moves, pinned, pinRay, validMask);
    genBishops(b, moves, pinned, pinRay, validMask);
    genQueens(b, moves, pinned, pinRay, validMask);
}

// Keep Check function for external query if needed
bool MoveGenerator::inCheck(const Board& b, bool white) {
     int p = white ? WK : BK;
     int ksq = get_lsb(b.pieces[p]);
     return attackersTo(ksq, b, !white) != 0;
}

