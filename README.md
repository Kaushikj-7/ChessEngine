# Chess Engine

A simple chess engine written in C++.

## Features

- **Board Representation**: 8x8 array-based board representation.
- **Move Generation**: Generates pseudo-legal moves for all pieces (Pawns, Knights, Bishops, Rooks, Queens, King).
- **Search**: Alpha-Beta pruning search algorithm found in `search.cpp`.
- **Evaluation**: Material-based evaluation function in `eval.cpp`.
- **Move Making/Unmaking**: Support for making and unmaking moves on the board.

## Building the Project

You can build the project using `g++` or `cmake`.

### Using g++

```bash
g++ main.cpp board.cpp move.cpp movegen.cpp search.cpp eval.cpp -o chess
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

## Running

After building, run the executable:

On Windows:

```powershell
.\chess.exe
```

On Linux/macOS:

```bash
./chess
```

## File Structure

- `main.cpp`: Entry point of the application.
- `board.h/cpp`: Board state and manipulation.
- `move.h/cpp`: Move structure definition.
- `movegen.h/cpp`: Move generation logic.
- `search.h/cpp`: Search algorithms (Alpha-Beta).
- `eval.h/cpp`: Static board evaluation.

## Version

Version 1.0
