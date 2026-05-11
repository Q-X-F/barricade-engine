#pragma once

#include "barricade_state.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <string>
#include <vector>

struct BarricadeEngine {
    struct SearchResult {
        bool has_move = false;
        BarricadeState state;
        std::string move_notation;
        int score = 0;
        int depth_searched = 0;
        int nodes_searched = 0;
        bool timed_out = false;
        double time_limit_seconds = 0.0;
    };

    int max_depth = 3;

    explicit BarricadeEngine(int depth = 3) : max_depth(depth) {}

    SearchResult best_move(const BarricadeState& state) const {
        bool timed_out = false;
        return search_to_depth(state, max_depth, nullptr, timed_out);
    }

    SearchResult best_move_with_time_limit(const BarricadeState& state, double seconds) const {
        if (seconds <= 0.0) {
            return best_move(state);
        }

        const Clock::time_point deadline = Clock::now() + std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(seconds));
        SearchResult best_completed;
        best_completed.time_limit_seconds = seconds;

        std::vector<BarricadeState> next_states = ordered_next_states(state);
        if (next_states.empty()) {
            best_completed.score = evaluate(state, 0);
            best_completed.nodes_searched = 1;
            return best_completed;
        }

        best_completed.has_move = true;
        best_completed.state = next_states.front();
        best_completed.move_notation = state.move_notation_to(best_completed.state);
        best_completed.score = evaluate(best_completed.state, 0);

        int total_nodes = 0;
        for (int depth = 1; depth <= max_depth; ++depth) {
            bool timed_out = false;
            SearchResult current = search_to_depth(state, depth, &deadline, timed_out);
            total_nodes += current.nodes_searched;

            if (timed_out) {
                best_completed.timed_out = true;
                best_completed.nodes_searched = total_nodes;
                return best_completed;
            }

            current.time_limit_seconds = seconds;
            current.nodes_searched = total_nodes;
            best_completed = current;
        }

        return best_completed;
    }

    SearchResult search_to_depth(const BarricadeState& state, int depth,
                                 const std::chrono::steady_clock::time_point* deadline,
                                 bool& timed_out) const {
        SearchResult result;
        result.depth_searched = depth;

        std::vector<BarricadeState> next_states = ordered_next_states(state);
        if (deadline_reached(deadline)) {
            timed_out = true;
            result.timed_out = true;
            return result;
        }
        if (next_states.empty()) {
            result.score = evaluate(state, 0);
            result.nodes_searched = 1;
            return result;
        }

        const bool maximize = state.p1_to_move;
        int alpha = -INF;
        int beta = INF;
        int best_score = maximize ? -INF : INF;
        BarricadeState best_state = next_states.front();

        for (const BarricadeState& next : next_states) {
            int nodes = 0;
            const int score = minimax(next, depth - 1, alpha, beta, nodes, deadline, timed_out);
            result.nodes_searched += nodes;
            if (timed_out) {
                result.timed_out = true;
                return result;
            }

            if ((maximize && score > best_score) || (!maximize && score < best_score)) {
                best_score = score;
                best_state = next;
            }

            if (maximize) {
                alpha = std::max(alpha, best_score);
            } else {
                beta = std::min(beta, best_score);
            }
        }

        result.has_move = true;
        result.state = best_state;
        result.move_notation = state.move_notation_to(best_state);
        result.score = best_score;
        return result;
    }

    static int evaluate(const BarricadeState& state, int depth_remaining = 0) {
        if (state.p1_won()) return WIN_SCORE + depth_remaining;
        if (state.p2_won()) return -WIN_SCORE - depth_remaining;

        const int p1_dist = state.p1_shortest_path_length();
        const int p2_dist = state.p2_shortest_path_length();
        const int safe_p1_dist = p1_dist >= 0 ? p1_dist : BarricadeState::NUM_SQUARES;
        const int safe_p2_dist = p2_dist >= 0 ? p2_dist : BarricadeState::NUM_SQUARES;

        const int path_score = 100 * (safe_p2_dist - safe_p1_dist);
        //const int progress_score = 10 * (BarricadeState::row(state.p1) - (BarricadeState::N - 1 - BarricadeState::row(state.p2)));
        const int barricade_score = 6 * (state.p1_barricades - state.p2_barricades);
        return path_score + barricade_score;
    }

private:
    using Clock = std::chrono::steady_clock;

    static constexpr int INF = std::numeric_limits<int>::max() / 4;
    static constexpr int WIN_SCORE = 1'000'000;

    static bool deadline_reached(const Clock::time_point* deadline) {
        return deadline != nullptr && Clock::now() >= *deadline;
    }

    int minimax(const BarricadeState& state, int depth, int alpha, int beta, int& nodes,
                const Clock::time_point* deadline, bool& timed_out) const {
        if (deadline_reached(deadline)) {
            timed_out = true;
            return evaluate(state, depth);
        }

        ++nodes;
        if (depth <= 0 || state.terminal()) {
            return evaluate(state, depth);
        }

        std::vector<BarricadeState> next_states = ordered_next_states(state);
        if (deadline_reached(deadline)) {
            timed_out = true;
            return evaluate(state, depth);
        }
        if (next_states.empty()) {
            return evaluate(state, depth);
        }

        if (state.p1_to_move) {
            int best = -INF;
            for (const BarricadeState& next : next_states) {
                best = std::max(best, minimax(next, depth - 1, alpha, beta, nodes, deadline, timed_out));
                if (timed_out) return best;
                alpha = std::max(alpha, best);
                if (alpha >= beta) break;
            }
            return best;
        } else {
            int best = INF;
            for (const BarricadeState& next : next_states) {
                best = std::min(best, minimax(next, depth - 1, alpha, beta, nodes, deadline, timed_out));
                if (timed_out) return best;
                beta = std::min(beta, best);
                if (alpha >= beta) break;
            }
            return best;
        }
    }

    static std::vector<BarricadeState> ordered_next_states(const BarricadeState& state) {
        std::vector<BarricadeState> next_states = state.legal_next_states();
        const bool maximize = state.p1_to_move;
        std::sort(next_states.begin(), next_states.end(),
                  [maximize](const BarricadeState& a, const BarricadeState& b) {
                      const int a_score = evaluate(a);
                      const int b_score = evaluate(b);
                      return maximize ? a_score > b_score : a_score < b_score;
                  });
        return next_states;
    }
};
