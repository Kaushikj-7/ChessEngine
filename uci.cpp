#include "uci.h"
#include "board.h"
#include "search.h"
#include "movegen.h"
#include "attacks.h"
#include "rays.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

void parsePosition(std::istringstream& is, Board& b) {
    std::string token, fen;
    is >> token;
    
    if (token == "startpos") {
        b = Board(); // Reset to start
        is >> token; // consume "moves" if present
    } else if (token == "fen") {
        // FEN parsing is complex, skipping for now or assume internal simple setup?
        // Assuming "position startpos" is primary for now as per instructions "UCI interface" implies playing.
        // If FEN needed, we need FEN parser in Board.
        // For now, only startpos supported or partial fen?
        // Let's just handle startpos correctly.
        b = Board(); // Default safety
    }

    if (token == "moves") {
        std::string moveStr;
        while (is >> moveStr) {
            // Parse moveStr (e.g. "e2e4", "a7a8q")
            // Generate legal moves to find match
            std::vector<Move> moves;
            MoveGenerator::generate(b, moves);
            
            int f = (moveStr[0] - 'a') + (moveStr[1] - '1') * 8;
            int t = (moveStr[2] - 'a') + (moveStr[3] - '1') * 8;
            int promo = -1;
            if (moveStr.length() > 4) {
                // promotion
                char p = moveStr[4];
                if (b.whiteToMove) {
                    if (p == 'q') promo = WQ;
                    if (p == 'r') promo = WR;
                    if (p == 'b') promo = WB;
                    if (p == 'n') promo = WN;
                } else {
                    if (p == 'q') promo = BQ;
                    if (p == 'r') promo = BR;
                    if (p == 'b') promo = BB;
                    if (p == 'n') promo = BN;
                }
            }

            bool found = false;
            for (const auto& m : moves) {
                if (m.from == f && m.to == t) {
                    // Check promo
                    bool match = true;
                    if (promo != -1) {
                         if (m.promotion != promo) match = false;
                    } else {
                         // If move is promotion but we didn't specify, it's invalid input or default Queen?
                         // UCI always specifies promo char.
                         // But if move IS a promo in engine, we must match it.
                         if (m.promotion != -1 && m.promotion != CASTLE && m.promotion != ENPASSANT) {
                             // Engine move is promo, user string has no promo char?
                             match = false;
                         }
                    }
                    if (match) {
                        b.makeMove(m);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                // std::cerr << "Invalid move: " << moveStr << "\n";
            }
        }
    }
}

void parseGo(std::istringstream& is, Board& b) {
    // Parse params like "depth 6", "wtime 12000" etc.
    std::string token;
    int depth = 5; // Default

    while (is >> token) {
        if (token == "depth") is >> depth;
        // else if (token == "wtime") ...
    }

    // Run search
    Move best = Search::findBestMove(b, depth);
    
    // Output format: e2e4 or a7a8q
    std::string promo = "";
    if (best.promotion != -1 && best.promotion != CASTLE && best.promotion != ENPASSANT) {
        int p = best.promotion;
        // Map back to char
        // WP=0... BN=7, BB=8, BR=9, BQ=10, BK=11
        // White: WN=1, WB=2, WR=3, WQ=4
        if (p == WN || p == BN) promo = "n";
        else if (p == WB || p == BB) promo = "b";
        else if (p == WR || p == BR) promo = "r";
        else if (p == WQ || p == BQ) promo = "q";
    }

    std::cout << "bestmove " << (char)('a' + (best.from % 8)) << (char)('1' + (best.from / 8))
              << (char)('a' + (best.to % 8)) << (char)('1' + (best.to / 8))
              << promo << "\n";
}

void UCILoop() {
    Board board;
    std::string line, token;

    std::cout << "id name Ethreal Engine\n";
    std::cout << "id author AutoAgent\n";
    std::cout << "uciok\n";

    while (std::getline(std::cin, line)) {
        std::istringstream is(line);
        token.clear();
        is >> token;

        if (token == "isready") {
            std::cout << "readyok\n";
        } else if (token == "position") {
            parsePosition(is, board);
        } else if (token == "go") {
            parseGo(is, board);
        } else if (token == "quit") {
            break;
        } else if (token == "uci") {
             std::cout << "id name Ethreal Engine\n";
             std::cout << "id author AutoAgent\n";
             std::cout << "uciok\n";
        } else if (token == "print") {
            board.print();
        }
    }
}
