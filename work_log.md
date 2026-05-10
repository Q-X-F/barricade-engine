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

## 2026-05-10

- Step 1: Reviewed `rules.txt`, `barricade_state.hpp`, and this work log before adding the engine.
- Decided to keep the rules implementation in `BarricadeState` as the source of truth and add minimax separately, with only small state-query helpers if needed for evaluation.
- Step 2: Added public shortest-path helper methods for p1 and p2 so the engine can evaluate positions using the same edge-blocking logic as move generation.
- Added a small reserve in `legal_next_states()` to avoid repeated vector growth during search.
- Step 3: Added `barricade_engine.hpp` with a depth-limited alpha-beta minimax engine.
- The engine uses p1-perspective scoring: terminal wins dominate, then shortest path advantage, pawn progress, and remaining barricade count.
- Added move ordering based on the static evaluator so alpha-beta can prune earlier, while still relying on `BarricadeState::legal_next_states()` for legal move generation.
- Step 4: Smoke-tested the engine with `clang++ -std=c++17 -Wall -Wextra -pedantic`.
- Depth-1 opening search returned `has_move=1`, `score=110`, and `nodes=131`.
- Step 5: Re-tested at depth 2; opening search returned `has_move=1`, `score=0`, and `nodes=392`.

## 2026-05-10

- Step 1: Read the appended notation rules in `rules.txt`.
- Interpreted square move notation as a two-character destination square such as `e2`.
- Interpreted barricade notation as `h` or `v` followed by a square name; the horizontal example `hd1` maps directly to the existing internal horizontal barricade anchor.
- Step 2: Added notation helpers to `BarricadeState`: `square_notation()`, `barricade_notation()`, and `move_notation_to()`.
- Added `BarricadeState::make_move(const std::string&)`, which parses notation, checks the existing legal move or barricade-placement rules, mutates the state only on success, advances the turn, and returns whether the move was made.
- Step 3: Updated `BarricadeEngine::SearchResult` with `move_notation`.
- Set `move_notation` in `best_move()` by comparing the input state with the selected legal child state.
- Step 4: Smoke-tested notation support with `clang++ -std=c++17 -Wall -Wextra -pedantic`.
- Verified `make_move("e2")` moves p1 and flips the turn, `make_move("hd1")` places a barricade and decrements p1's count, illegal notation leaves state unchanged, and the engine's depth-1 `move_notation` can be replayed successfully.
- Step 5: Reviewed the final diff and confirmed the intended changed files are `barricade_state.hpp`, `barricade_engine.hpp`, and `work_log.md`.
