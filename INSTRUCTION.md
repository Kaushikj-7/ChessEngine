# Ethreal Engine v3.0: CI/CD & Algorithmic Profiling Directive

## 1. Zero-Regression Testing (The Pre-Push Protocol)
Before any commit is pushed to the main branch, the engine must prove it has not mathematically degraded. Chess engines do not use standard "unit tests" for their core logic; they use Perft (Performance Test).

### The Perft Standard
Perft walks the move generation tree to a specific depth and counts the exact number of leaf nodes. If your engine generates 119,060,324 nodes at Depth 6 from the starting position, your move generator is 100% bug-free. If it generates 119,060,323, you have a critical failure in your bitwise logic.

### The Pre-Push Pipeline
- **Symmetry Tests**: Evaluate a position, flip the board (colors reversed), and evaluate again. The score must be perfectly inverted (Eval_white = -Eval_black).
- **Mate-in-X Solvers**: A suite of 50 forced-mate tactics. The engine must find the correct mate within a hard time limit of 0.5 seconds to verify search algorithms aren't bypassing critical lines.
- **Memory Leak Check**: Run the test suite through Valgrind. A single byte of leaked memory in the Transposition Table results in a rejected build.

## 2. Safe Algorithmic Ablation Studies (Code Architecture)
To test v3.0 features (like Zobrist Hashing) against v2.0 without "bugging" the codebase, do not comment out old code. You will build an Ablation Framework.

### Avoid Runtime Polymorphism
Do not use base classes and inheritance for algorithms in the search tree. It is too slow for C++.

### Implementation Strategy
Use preprocessor directives (`#ifdef`) or C++ Templates to switch algorithms at compile time.
- **Build Target A**: `chess_v2.exe` (Standard Alpha-Beta)
- **Build Target B**: `chess_v3_exp.exe` (PVS + Zobrist)

### The Metric
Run both executables side-by-side using an external automated referee like `Cutechess-cli`. Have them play 1,000 bullet games against each other. If v3 does not score a mathematically significant Elo gain (e.g., +50 Elo), the algorithm is discarded.

## 3. Industry-Standard Metric Collection
Corporate teams do not look at code; they look at telemetry. When comparing algorithms, document the following four pillars:

### A. Throughput (Nodes Per Second - NPS)
- **What it is**: Pure hardware utilization. How fast is your C++.
- **How to measure**: `Total Nodes Evaluated / Total Time in Seconds`.
- **The Goal**: A clean Bitboard engine on a modern CPU should clear 1,000,000 to 5,000,000 NPS on a single thread.

### B. Algorithmic Efficiency (Branching Factor)
- **What it is**: The mathematical effectiveness of your heuristics (MVV-LVA, LMR).
- **How to measure**: Compare the total nodes searched to reach Depth 8 using standard Alpha-Beta vs. Principal Variation Search (PVS).
- **The Goal**: The algorithm that reaches Depth 8 evaluating the fewest total nodes is mathematically superior, even if the NPS drops slightly.

### C. Hardware Latency (Cache Misses)
- **What it is**: The hidden killer of high-performance C++. When your Transposition Table gets too large, your CPU wastes cycles fetching data from RAM instead of L1/L2 cache.
- **How to measure**: Use Linux tools like `perf stat ./chess.exe`.
- **The Goal**: Track the `L1-dcache-load-misses` percentage. If implementing Zobrist Hashing spikes your cache misses from 1% to 15%, your hash table is misaligned in memory.

### D. Time to Depth (TTD)
- **What it is**: The ultimate user-facing metric. How quickly does the engine achieve a confident evaluation?
- **How to measure**: Log the exact millisecond timestamp when the engine completes Depth 1, Depth 2, etc., up to Depth 10.
- **The Goal**: Prove that Iterative Deepening combined with Transposition Tables drastically flattens the O(b^d) exponential time curve of the search tree.
