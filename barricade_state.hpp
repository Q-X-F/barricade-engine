#pragma once

#include <array>
#include <cstdlib>
#include <cstdint>
#include <initializer_list>
#include <queue>
#include <utility>
#include <vector>

struct BarricadeState {
    static constexpr int N = 9;
    static constexpr int NUM_SQUARES = N * N;

    // Squares are encoded as row * 9 + col, with col 0 == a and row 0 == 1.
    // Vertical barricade:   blocks two east-west adjacencies at (col,row) and (col,row+1).
    // Horizontal barricade: blocks two north-south adjacencies at (col,row) and (col+1,row).
    // In both cases col,row identify the southwest square of the interior vertex.
    struct Barricade {
        int col = 0;
        int row = 0;
        bool horizontal = false;

        bool operator==(const Barricade& other) const {
            return col == other.col && row == other.row && horizontal == other.horizontal;
        }
    };

    int p1 = square(4, 0);
    int p2 = square(4, 8);
    bool p1_to_move = true;
    int p1_barricades = 10;
    int p2_barricades = 10;
    std::vector<Barricade> barricades;

    BarricadeState() = default;
    BarricadeState(const BarricadeState&) = default;
    BarricadeState& operator=(const BarricadeState&) = default;

    static constexpr int square(int col, int row) { return row * N + col; }
    static constexpr int col(int sq) { return sq % N; }
    static constexpr int row(int sq) { return sq / N; }

    bool p1_won() const { return row(p1) == N - 1; }
    bool p2_won() const { return row(p2) == 0; }
    bool terminal() const { return p1_won() || p2_won(); }

    std::vector<BarricadeState> legal_next_states() const {
        std::vector<BarricadeState> out;
        if (terminal()) return out;

        for (int dst : legal_move_squares()) {
            BarricadeState next(*this);
            (p1_to_move ? next.p1 : next.p2) = dst;
            next.p1_to_move = !p1_to_move;
            out.push_back(std::move(next));
        }

        const int remaining = p1_to_move ? p1_barricades : p2_barricades;
        if (remaining > 0) {
            for (int r = 0; r < N - 1; ++r) {
                for (int c = 0; c < N - 1; ++c) {
                    for (bool horizontal : {false, true}) {
                        Barricade b{c, r, horizontal};
                        if (!can_place_barricade(b)) continue;

                        BarricadeState next(*this);
                        next.barricades.push_back(b);
                        if (p1_to_move) {
                            --next.p1_barricades;
                        } else {
                            --next.p2_barricades;
                        }
                        next.p1_to_move = !p1_to_move;
                        out.push_back(std::move(next));
                    }
                }
            }
        }

        return out;
    }

private:
    static constexpr std::array<int, 4> DC{0, 1, 0, -1};
    static constexpr std::array<int, 4> DR{1, 0, -1, 0};

    bool in_bounds(int c, int r) const {
        return 0 <= c && c < N && 0 <= r && r < N;
    }

    bool edge_blocked(int a, int b) const {
        const int ac = col(a), ar = row(a);
        const int bc = col(b), br = row(b);
        for (const Barricade& wall : barricades) {
            if (wall.horizontal) {
                if (ar + 1 == br || br + 1 == ar) {
                    const int south_row = ar < br ? ar : br;
                    if (south_row == wall.row && (ac == wall.col || ac == wall.col + 1) && ac == bc) {
                        return true;
                    }
                }
            } else {
                if (ac + 1 == bc || bc + 1 == ac) {
                    const int west_col = ac < bc ? ac : bc;
                    if (west_col == wall.col && (ar == wall.row || ar == wall.row + 1) && ar == br) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    std::vector<int> legal_move_squares() const {
        std::vector<int> moves;
        const int me = p1_to_move ? p1 : p2;
        const int opp = p1_to_move ? p2 : p1;

        const int mc = col(me), mr = row(me);
        for (int dir = 0; dir < 4; ++dir) {
            const int nc = mc + DC[dir], nr = mr + DR[dir];
            if (!in_bounds(nc, nr)) continue;

            const int adjacent = square(nc, nr);
            if (edge_blocked(me, adjacent)) continue;

            if (adjacent != opp) {
                moves.push_back(adjacent);
                continue;
            }

            const int jc = nc + DC[dir], jr = nr + DR[dir];
            const bool straight_jump =
                in_bounds(jc, jr) && !edge_blocked(adjacent, square(jc, jr));
            if (straight_jump) {
                moves.push_back(square(jc, jr));
                continue;
            }

            for (int side : {1, 3}) {
                const int side_dir = (dir + side) % 4;
                const int lc = nc + DC[side_dir], lr = nr + DR[side_dir];
                if (!in_bounds(lc, lr)) continue;

                const int landing = square(lc, lr);
                if (!edge_blocked(adjacent, landing)) {
                    moves.push_back(landing);
                }
            }
        }

        return moves;
    }

    bool can_place_barricade(const Barricade& b) const {
        if (b.col < 0 || b.col >= N - 1 || b.row < 0 || b.row >= N - 1) {
            return false;
        }

        for (const Barricade& existing : barricades) {
            if (existing.col == b.col && existing.row == b.row) {
                return false; // same vertex: overlap if parallel, crossing if perpendicular
            }
            if (existing.horizontal == b.horizontal) {
                if (b.horizontal && existing.row == b.row && std::abs(existing.col - b.col) == 1) {
                    return false; // same north-south edge would be occupied twice
                }
                if (!b.horizontal && existing.col == b.col && std::abs(existing.row - b.row) == 1) {
                    return false; // same east-west edge would be occupied twice
                }
            }
        }

        BarricadeState next(*this);
        next.barricades.push_back(b);
        return next.has_path_to_goal(p1, true) && next.has_path_to_goal(p2, false);
    }

    bool has_path_to_goal(int start, bool for_p1) const {
        std::array<bool, NUM_SQUARES> seen{};
        std::queue<int> q;
        seen[start] = true;
        q.push(start);

        while (!q.empty()) {
            const int cur = q.front();
            q.pop();
            if ((for_p1 && row(cur) == N - 1) || (!for_p1 && row(cur) == 0)) {
                return true;
            }

            const int cc = col(cur), cr = row(cur);
            for (int dir = 0; dir < 4; ++dir) {
                const int nc = cc + DC[dir], nr = cr + DR[dir];
                if (!in_bounds(nc, nr)) continue;

                const int nxt = square(nc, nr);
                if (!seen[nxt] && !edge_blocked(cur, nxt)) {
                    seen[nxt] = true;
                    q.push(nxt);
                }
            }
        }

        return false;
    }
};