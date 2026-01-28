#include "movegen.h"

static bool isWhite(char p) { return p >= 'A' && p <= 'Z'; }
static bool isBlack(char p) { return p >= 'a' && p <= 'z'; }

static void addSlidingMoves(const Board& board, std::vector<Move>& moves,
                            int r, int f, bool white,
                            const int dr[], const int df[], int count) {
    for (int d = 0; d < count; ++d) {
        int nr = r + dr[d], nf = f + df[d];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            int to = nr * 8 + nf;
            char target = board.squares[to];

            if (target == '.') {
                moves.emplace_back(r * 8 + f, to);
            } else {
                if ((white && isBlack(target)) || (!white && isWhite(target)))
                    moves.emplace_back(r * 8 + f, to, target);
                break;
            }
            nr += dr[d];
            nf += df[d];
        }
    }
}

void MoveGenerator::generate(const Board& board, std::vector<Move>& moves) {
    moves.clear();

    for (int i = 0; i < 64; ++i) {
        char p = board.squares[i];
        if (p == '.') continue;

        bool white = isWhite(p);
        if (white != board.whiteToMove) continue;

        int r = i / 8, f = i % 8;

        // ===== PAWNS =====
        if (p == 'P') {
            int one = i - 8;
            if (one >= 0 && board.squares[one] == '.')
                moves.emplace_back(i, one);

            if (r == 6 && board.squares[i - 8] == '.' && board.squares[i - 16] == '.')
                moves.emplace_back(i, i - 16);

            if (f > 0 && isBlack(board.squares[i - 9]))
                moves.emplace_back(i, i - 9, board.squares[i - 9]);
            if (f < 7 && isBlack(board.squares[i - 7]))
                moves.emplace_back(i, i - 7, board.squares[i - 7]);
        }

        if (p == 'p') {
            int one = i + 8;
            if (one < 64 && board.squares[one] == '.')
                moves.emplace_back(i, one);

            if (r == 1 && board.squares[i + 8] == '.' && board.squares[i + 16] == '.')
                moves.emplace_back(i, i + 16);

            if (f > 0 && isWhite(board.squares[i + 7]))
                moves.emplace_back(i, i + 7, board.squares[i + 7]);
            if (f < 7 && isWhite(board.squares[i + 9]))
                moves.emplace_back(i, i + 9, board.squares[i + 9]);
        }

        // ===== KNIGHTS =====
        if (p == 'N' || p == 'n') {
            const int dr[8] = {-2,-2,-1,-1,1,1,2,2};
            const int df[8] = {-1,1,-2,2,-2,2,-1,1};

            for (int k = 0; k < 8; ++k) {
                int nr = r + dr[k], nf = f + df[k];
                if (nr < 0 || nr > 7 || nf < 0 || nf > 7) continue;
                int to = nr * 8 + nf;
                char target = board.squares[to];
                if (target == '.') {
                    moves.emplace_back(i, to);
                } else if ((white && isBlack(target)) || (!white && isWhite(target))) {
                    moves.emplace_back(i, to, target);
                }
            }
        }

        // ===== SLIDERS =====
        if (p == 'B' || p == 'b' || p == 'Q' || p == 'q') {
            const int dr[4] = {-1,-1,1,1};
            const int df[4] = {-1,1,-1,1};
            addSlidingMoves(board, moves, r, f, white, dr, df, 4);
        }

        if (p == 'R' || p == 'r' || p == 'Q' || p == 'q') {
            const int dr[4] = {-1,1,0,0};
            const int df[4] = {0,0,-1,1};
            addSlidingMoves(board, moves, r, f, white, dr, df, 4);
        }

        // ===== KING =====
        if (p == 'K' || p == 'k') {
            for (int dr = -1; dr <= 1; ++dr)
                for (int df = -1; df <= 1; ++df) {
                    if (dr == 0 && df == 0) continue;
                    int nr = r + dr, nf = f + df;
                    if (nr < 0 || nr > 7 || nf < 0 || nf > 7) continue;
                    int to = nr * 8 + nf;
                    char target = board.squares[to];
                    if (target == '.') {
                        moves.emplace_back(i, to);
                    } else if ((white && isBlack(target)) || (!white && isWhite(target))) {
                        moves.emplace_back(i, to, target);
                    }
                }
        }
    }
}
static bool attacksSquare(const Board& board, int sq, bool byWhite);

bool MoveGenerator::inCheck(const Board& board, bool white) {
    int kingSq = -1;
    char k = white ? 'K' : 'k';

    for (int i = 0; i < 64; ++i)
        if (board.squares[i] == k) kingSq = i;

    return attacksSquare(board, kingSq, !white);
}
static bool attacksSquare(const Board& board, int sq, bool byWhite) {
    int r = sq / 8, f = sq % 8;

    // Pawns
    int dir = byWhite ? -1 : 1;
    int pr = r + dir;
    if (pr >= 0 && pr < 8) {
        if (f > 0 && board.squares[pr*8 + f - 1] == (byWhite ? 'P' : 'p')) return true;
        if (f < 7 && board.squares[pr*8 + f + 1] == (byWhite ? 'P' : 'p')) return true;
    }

    // Knights
    const int kdr[8] = {-2,-2,-1,-1,1,1,2,2};
    const int kdf[8] = {-1,1,-2,2,-2,2,-1,1};
    for (int i = 0; i < 8; ++i) {
        int nr = r + kdr[i], nf = f + kdf[i];
        if (nr < 0 || nr > 7 || nf < 0 || nf > 7) continue;
        char p = board.squares[nr*8 + nf];
        if (p == (byWhite ? 'N' : 'n')) return true;
    }

    // Sliding: rook/queen
    const int rdr[4] = {-1,1,0,0};
    const int rdf[4] = {0,0,-1,1};
    for (int d = 0; d < 4; ++d) {
        int nr = r + rdr[d], nf = f + rdf[d];
        while (nr>=0 && nr<8 && nf>=0 && nf<8) {
            char p = board.squares[nr*8 + nf];
            if (p != '.') {
                if (p == (byWhite ? 'R':'r') || p == (byWhite ? 'Q':'q')) return true;
                break;
            }
            nr += rdr[d]; nf += rdf[d];
        }
    }

    // Sliding: bishop/queen
    const int bdr[4] = {-1,-1,1,1};
    const int bdf[4] = {-1,1,-1,1};
    for (int d = 0; d < 4; ++d) {
        int nr = r + bdr[d], nf = f + bdf[d];
        while (nr>=0 && nr<8 && nf>=0 && nf<8) {
            char p = board.squares[nr*8 + nf];
            if (p != '.') {
                if (p == (byWhite ? 'B':'b') || p == (byWhite ? 'Q':'q')) return true;
                break;
            }
            nr += bdr[d]; nf += bdf[d];
        }
    }

    // King
    for (int dr=-1; dr<=1; ++dr)
        for (int df=-1; df<=1; ++df) {
            if (!dr && !df) continue;
            int nr=r+dr, nf=f+df;
            if (nr<0||nr>7||nf<0||nf>7) continue;
            char p = board.squares[nr*8 + nf];
            if (p == (byWhite ? 'K':'k')) return true;
        }

    return false;
}
