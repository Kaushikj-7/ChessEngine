#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "board.h"
#include "search.h"
#include "attacks.h"
#include "rays.h"

struct MateTestCase {
    std::string fen;
    int depth;
    std::string expectedMove;
};

int main() {
    initRays();
    initAttackTables();

    std::vector<MateTestCase> tests = {
        // Mate in 1: Scholar's Mate
        {"r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4", 2, "h5f7"},
        // Mate in 1: Back rank mate
        {"6k1/5ppp/8/8/8/8/5PPP/3R2K1 w - - 0 1", 2, "d1d8"},
        // Mate in 2: Waiting move
        {"1k6/R7/1K6/8/8/8/8/8 w - - 0 1", 4, "a7h7"}
    };

    int passed = 0;
    for (const auto& test : tests) {
        Board b;
        b.loadFen(test.fen);
        
        auto start = std::chrono::high_resolution_clock::now();
        Move best = Search::findBestMove(b, test.depth);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        std::string moveStr = "";
        if (best.from != -1) {
            moveStr += (char)('a' + (best.from % 8));
            moveStr += (char)('1' + (best.from / 8));
            moveStr += (char)('a' + (best.to % 8));
            moveStr += (char)('1' + (best.to / 8));
        } else {
            moveStr = "none";
        }

        std::cout << "FEN: " << test.fen << std::endl;
        std::cout << "Expected: " << test.expectedMove << ", Got: " << moveStr;
        std::cout << " (Time: " << diff.count() << "s)" << std::endl;

        if (moveStr == test.expectedMove) {
            passed++;
            std::cout << "PASSED" << std::endl << std::endl;
        } else {
            std::cout << "FAILED" << std::endl << std::endl;
        }
    }

    std::cout << "Passed " << passed << "/" << tests.size() << " tests." << std::endl;

    return (passed == (int)tests.size()) ? 0 : 1;
}
