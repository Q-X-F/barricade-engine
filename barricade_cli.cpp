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
        << "  time <seconds>  Set a best/playbest time limit; use time off to disable\n"
        << "  moves           Print the number of legal next states\n"
        << "  status          Print the current state\n"
        << "  reset           Reset to the starting position\n"
        << "  help            Print this help\n"
        << "  quit            Exit\n";
}

void print_status(const BarricadeState& state, int depth, double time_limit_seconds) {
    std::cout << "p1=" << BarricadeState::square_notation(state.p1)
              << " p2=" << BarricadeState::square_notation(state.p2)
              << " turn=" << (state.p1_to_move ? "p1" : "p2")
              << " p1_barricades=" << state.p1_barricades
              << " p2_barricades=" << state.p2_barricades
              << " placed=" << state.barricades.size()
              << " depth=" << depth
              << " time_limit=";
    if (time_limit_seconds > 0.0) {
        std::cout << time_limit_seconds << "s\n";
    } else {
        std::cout << "off\n";
    }

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

bool parse_time_limit(const std::string& text, double& seconds, bool& disable) {
    std::istringstream in(text);
    std::string token;
    std::string extra;
    if (!(in >> token) || (in >> extra)) {
        return false;
    }

    if (token == "off" || token == "none" || token == "0") {
        disable = true;
        seconds = 0.0;
        return true;
    }

    std::istringstream number_stream(token);
    double parsed = 0.0;
    if (!(number_stream >> parsed) || !number_stream.eof() || parsed <= 0.0) {
        return false;
    }

    disable = false;
    seconds = parsed;
    return true;
}

void print_best_move(const BarricadeState& state, int depth, double time_limit_seconds,
                     bool apply, BarricadeState& mutable_state) {
    if (state.terminal()) {
        std::cout << "No best move: terminal state already reached.\n";
        return;
    }

    BarricadeEngine engine(depth);
    const BarricadeEngine::SearchResult result =
        time_limit_seconds > 0.0 ? engine.best_move_with_time_limit(state, time_limit_seconds)
                                 : engine.best_move(state);
    if (!result.has_move) {
        std::cout << "No legal move found.\n";
        return;
    }

    std::cout << "best=" << result.move_notation
              << " score=" << result.score
              << " depth=" << result.depth_searched
              << " nodes=" << result.nodes_searched;
    if (time_limit_seconds > 0.0) {
        std::cout << " timed_out=" << (result.timed_out ? "yes" : "no")
                  << " limit=" << time_limit_seconds << "s";
    }
    std::cout << '\n';

    if (apply) {
        mutable_state = result.state;
        std::cout << "Applied " << result.move_notation << ".\n";
    }
}

} // namespace

int main() {
    BarricadeState state;
    int depth = 3;
    double time_limit_seconds = 0.0;

    std::cout << "Barricade manual tester. Type help for commands.\n";
    print_status(state, depth, time_limit_seconds);

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
            print_status(state, depth, time_limit_seconds);
            continue;
        }

        if (command == "reset") {
            state = BarricadeState{};
            std::cout << "Board reset.\n";
            print_status(state, depth, time_limit_seconds);
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

        if (command == "time") {
            std::string rest;
            std::getline(command_stream, rest);
            double parsed_seconds = 0.0;
            bool disable = false;
            if (!parse_time_limit(rest, parsed_seconds, disable)) {
                std::cout << "Usage: time <positive seconds> or time off\n";
                continue;
            }

            time_limit_seconds = disable ? 0.0 : parsed_seconds;
            if (time_limit_seconds > 0.0) {
                std::cout << "Time limit set to " << time_limit_seconds << " seconds.\n";
            } else {
                std::cout << "Time limit disabled.\n";
            }
            continue;
        }

        if (command == "best") {
            print_best_move(state, depth, time_limit_seconds, false, state);
            continue;
        }

        if (command == "playbest") {
            print_best_move(state, depth, time_limit_seconds, true, state);
            print_status(state, depth, time_limit_seconds);
            continue;
        }

        if (state.terminal()) {
            std::cout << "Move rejected: terminal state already reached. Use reset to start over.\n";
            continue;
        }

        if (state.make_move(command)) {
            std::cout << "Applied " << command << ".\n";
            print_status(state, depth, time_limit_seconds);
        } else {
            std::cout << "Illegal or invalid move: " << command << '\n';
        }
    }

    return EXIT_SUCCESS;
}
