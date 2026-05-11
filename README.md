# Barricade (Quoridor) Engine

This project is a C++ engine for the board game Quoridor. Quoridor is now hosted as the online game Barricade at [barricade.gg](barricade.gg).
The engine implements the game state, legal move generation, move notation, and a
minimax-based engine that can suggest moves from a position.

This is also my first project using an AI agent. The implementation was built
incrementally with the agent helping write code, run checks, and create a small
command-line interface for manual testing.

## Files

- `rules.txt`: rules and notation for Barricade.
- `barricade_state.hpp`: board state, legal move generation, move application,
  terminal-state checks, and notation helpers.
- `barricade_engine.hpp`: minimax engine with alpha-beta pruning.
- `barricade_cli.cpp`: standard input/output CLI for manually testing the game
  state and engine.

## Download

Clone the repository:

```sh
git clone <repository-url>
cd barricade
```

If you downloaded the project as a ZIP file, unzip it and open a terminal in the
project directory.

## Build The CLI

You need a C++17 compiler such as `clang++` or `g++`.

For normal use, build with compiler optimizations enabled:

```sh
clang++ -std=c++17 -O2 -Wall -Wextra -pedantic barricade_cli.cpp -o barricade_cli
```

Or with `g++`:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -pedantic barricade_cli.cpp -o barricade_cli
```

## Run

```sh
./barricade_cli
```

The CLI starts from the initial Barricade position. Type `help` to see available
commands.

Common commands:

```text
e2          Make a move using Barricade notation
hd1         Place a horizontal barricade
best        Ask the engine for the best move
playbest    Ask the engine for the best move and apply it
depth 3     Set engine search depth
moves       Print the number of legal next states
status      Print the current state
reset       Reset to the starting position
quit        Exit
```

The CLI reports player positions, side to move, remaining barricades, placed
barricades, shortest-path distances, and whether a terminal state has been
reached.

## Performance Note

The engine is much faster when compiled with optimization enabled. Use `-O2` for
manual engine testing. Debug builds without optimization can be dramatically
slower at deeper search depths.
