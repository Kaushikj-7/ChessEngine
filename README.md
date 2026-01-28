# Ethreal Chess Engine

A sophisticated chess engine written in C++ using Bitboards.

## Features

### Core Architecture

- **Board Representation**: Bitboard-based representation (Little-Endian Rank-File Mapping).
- **Move Generation**: Strictly legal move generator.
  - Specialized logic for Pins, Checks, and Check Evasions.
  - Full support for Castling and En Passant with safety verification.
  - Pre-calculated Ray and Attack tables (`rays.cpp`, `attacks.cpp`).

### Search & Evaluation

- **Search Algorithm**: Alpha-Beta Pruning with Move Ordering.
  - **Move Ordering**: MVV-LVA (Most Valuable Victim - Least Valuable Aggressor), Killer Heuristic, History Heuristic.
  - **Quiescence Search**: Solves horizon effects by extending search for non-quiet positions (captures).
- **Evaluation**:
  - Piece-Square Tables (PST) for positional scoring.
  - Material imbalance calculation.

### Interface

- **UCI Protocol**: Fully compliant Universal Chess Interface (UCI).
  - Supports `position`, `go`, `isready`, `uci`, `quit` commands.
  - Compatible with standard GUIs (Arena, Fritz, Banksia, Lichess).

## Building the Project

### Using g++

```powershell
g++ -std=c++17 main.cpp uci.cpp board.cpp move.cpp movegen.cpp search.cpp eval.cpp attacks.cpp rays.cpp -o chess.exe
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

## Running

The engine runs in UCI mode by default.

On Windows:

```powershell
.\chess.exe
```

## File Structure

- `main.cpp`: Entry point (connects UCI to Engine).
- `uci.cpp`: UCI protocol loop and command parsing.
- `board.cpp`: Bitboards, state tracking (castle rights, ep), make/unmake.
- `movegen.cpp`: Legal move generation (Checks, Pins, Evasions).
- `search.cpp`: Alpha-Beta, Ordering, Quiescence.
- `eval.cpp`: Piece-Square Table evaluation.
- `attacks.cpp`: Pre-calculated sliding attack tables (Magic-style logic).
- `rays.cpp`: Pre-calculated directional rays.

## Version

**Version 2.0**

### Roadmap for v3.0

- [ ] Transposition Tables (Zobrist Hashing)
- [ ] Iterative Deepening & Time Management
- [ ] Principal Variation Search (PVS)
- [ ] Late Move Reduction (LMR)
