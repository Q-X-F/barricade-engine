#include "barricade_engine.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace {

void print_help() {
    std::cout
        << "Commands:\n"
        << "  <move>          Make a move using notation, e.g. e2, hd1, vd3\n"
        << "  best            Ask the engine for the best move at the current depth\n"
        << "  playbest        Ask the engine for the best move and apply it\n"
        << "  depth <n>       Set engine search depth to n\n"
        << "  moves           Print the number of legal next states\n"
        << "  status          Print the current state\n"
        << "  reset           Reset to the starting position\n"
        << "  help            Print this help\n"
        << "  quit            Exit\n";
}

void print_status(const BarricadeState& state, int depth) {
    std::cout << "p1=" << BarricadeState::square_notation(state.p1)
              << " p2=" << BarricadeState::square_notation(state.p2)
              << " turn=" << (state.p1_to_move ? "p1" : "p2")
              << " p1_barricades=" << state.p1_barricades
              << " p2_barricades=" << state.p2_barricades
              << " placed=" << state.barricades.size()
              << " depth=" << depth << '\n';

    if (!state.barricades.empty()) {
        std::cout << "barricades:";
        for (const BarricadeState::Barricade& b : state.barricades) {
            std::cout << ' ' << BarricadeState::barricade_notation(b);
        }
        std::cout << '\n';
    }

    std::cout << "shortest_paths p1=" << state.p1_shortest_path_length()
              << " p2=" << state.p2_shortest_path_length() << '\n';

    if (state.p1_won()) {
        std::cout << "terminal: p1 won\n";
    } else if (state.p2_won()) {
        std::cout << "terminal: p2 won\n";
    } else {
        std::cout << "terminal: no\n";
    }
}

bool parse_depth(const std::string& text, int& depth) {
    std::istringstream in(text);
    int parsed = 0;
    std::string extra;
    if (!(in >> parsed) || (in >> extra) || parsed < 1) {
        return false;
    }

    depth = parsed;
    return true;
}

void print_best_move(const BarricadeState& state, int depth, bool apply, BarricadeState& mutable_state) {
    if (state.terminal()) {
        std::cout << "No best move: terminal state already reached.\n";
        return;
    }

    BarricadeEngine engine(depth);
    const BarricadeEngine::SearchResult result = engine.best_move(state);
    if (!result.has_move) {
        std::cout << "No legal move found.\n";
        return;
    }

    std::cout << "best=" << result.move_notation
              << " score=" << result.score
              << " depth=" << result.depth_searched
              << " nodes=" << result.nodes_searched << '\n';

    if (apply) {
        mutable_state = result.state;
        std::cout << "Applied " << result.move_notation << ".\n";
    }
}

} // namespace

int main() {
    BarricadeState state;
    int depth = 3;

    std::cout << "Barricade manual tester. Type help for commands.\n";
    print_status(state, depth);

    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream command_stream(line);
        std::string command;
        command_stream >> command;

        if (command == "quit" || command == "exit") {
            break;
        }

        if (command == "help") {
            print_help();
            continue;
        }

        if (command == "status") {
            print_status(state, depth);
            continue;
        }

        if (command == "reset") {
            state = BarricadeState{};
            std::cout << "Board reset.\n";
            print_status(state, depth);
            continue;
        }

        if (command == "moves") {
            std::cout << "legal_next_states=" << state.legal_next_states().size() << '\n';
            continue;
        }

        if (command == "depth") {
            std::string rest;
            std::getline(command_stream, rest);
            int parsed_depth = 0;
            if (!parse_depth(rest, parsed_depth)) {
                std::cout << "Usage: depth <positive integer>\n";
                continue;
            }

            depth = parsed_depth;
            std::cout << "Depth set to " << depth << ".\n";
            continue;
        }

        if (command == "best") {
            print_best_move(state, depth, false, state);
            continue;
        }

        if (command == "playbest") {
            print_best_move(state, depth, true, state);
            print_status(state, depth);
            continue;
        }

        if (state.terminal()) {
            std::cout << "Move rejected: terminal state already reached. Use reset to start over.\n";
            continue;
        }

        if (state.make_move(command)) {
            std::cout << "Applied " << command << ".\n";
            print_status(state, depth);
        } else {
            std::cout << "Illegal or invalid move: " << command << '\n';
        }
    }

    return EXIT_SUCCESS;
}
