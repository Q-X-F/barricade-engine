# Barricade State Implementation Log

## 2026-05-02

- Read `/Users/chrisqiu/Downloads/cambridge/code/barricade/rules.txt`.
- Implemented `BarricadeState` in `barricade_state.hpp`.
- Represented board squares as `row * 9 + col`, where `a1` is `(0, 0)` and `e1` is `(4, 0)`.
- Stored player positions, turn, remaining barricade counts, and placed barricades.
- Added default copy constructor and assignment operator.
- Implemented `legal_next_states()` to generate:
  - normal adjacent moves,
  - straight jumps over the opponent,
  - L-shaped jumps when a straight jump is blocked or off-board,
  - legal barricade placements.
- Represented barricades by their interior vertex anchor plus orientation.
- Added checks for:
  - board bounds,
  - blocked edges,
  - barricade overlap,
  - barricade crossing,
  - one-edge overlap between same-orientation neighboring barricades,
  - remaining barricade count,
  - preserving at least one path to the goal row for both players.
- Used BFS for the post-placement path-existence rule, ignoring the other player's current square as required by the rules.
- Smoke-tested with:
  `clang++ -std=c++17 -Wall -Wextra -pedantic`
- Starting position generated `131` legal next states: `3` pawn moves plus `128` initial barricade placements.
