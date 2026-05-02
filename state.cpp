#include <iostream>
#include <cstdint>
#include <queue>
#include <chrono>

using namespace std;


/*
The game of barricade takes place on a board with 9x9 squares, with the 
columns named a, b, c, d, e, f, g, h, i from West to East, and the 
rows numbered 1, 2, 3, 4, 5, 6, 7, 8, 9 from South to North. 
An edge is defined as a side that is shared by two squares on the board, 
a vertex is defined as point that is the corner of four squares.

Each player occupies one square at any given time, with player 1 (p1) 
starting on e1 and player 2 (p2) starting on e9.
The game ends if p1 reaches a square on row 9, resulting in a win for p1, 
or if p2 reaches a square on row 1, resulting in a win for p2. 
There are no draws.

A barricade is a line whose length is two times the edge length of a 
square on the board. The board starts out with no barricades placed. 
All barricades on the board affect both players, no matter who placed them. 
Each player starts with a barricade count of 10.

Players take turns from the starting position, with p1 taking the first 
turn. On each turn a player must either move or place a barricade, but 
not both.

Rules for moving when it is p1's turn:
(1) p1 cannot move outside of the 9x9 board.
(2) If the square occupied by p1 share an edge with an empty square, 
    and that edge is not occupied by a barricade, p1 can move to the 
    empty square.
(3) If the square occupied by p1 share an edge with a square 
    occupied by p2, and that edge is not taken by a barricade:
    (a) p1 can jump over p2 in a straight line and land on 
        the empty square on the opposite side of p2, 
        provided the movement does not cross any barricades.
    (b) If a straight jump cannot be done, p1 can jump in 
        an L-shape and land on a square on either side of p2, 
        provided the movement does not cross any barricades.
(4) p1 cannot move to any square unspecified by the rules above.

To find the rules for moving when it is p2's turn, simply replace 
"p1" with "p2" and vice versa in the rules above.

Rules for placing a barricade:
(1) A barricade can only be placed on two edges that connect 
    to form a straight line. Such barricade is said to occupy both edges, 
    as well as the vertex between them.
(2) The outer border of the board are off-limits for barricades. This 
    rule is implicit by the definition of "edge" and "vertex" but restated 
    for clarity.
(3) Each edge can only be occupied by one barricade, therefore 
    barricades cannot overlap.
(4) Each vertex can only be occupied by one barricade, therefore 
    barricades cannot cross but can be placed in a T-shape.
(5) Once placed, a barricade is permanent for the duration of the game.
(6) When a player places a barricade, their barricade count decrements.
(7) A player whose barricade count is 0 cannot place any more barricades.
(8) At all points during the game, if either player is hypothetically 
    removed from the board the other player must have a sequence of moves 
    to win. More intuitively, it is not allowed to place a barricade that 
    blocks off all paths from p1 to row 9 or blocks off all paths from p2 
    to row 1.

*/

// Represents a board state
struct state {
    // There are 64 vertices in the board
    // a1-b1-a2-b2 vertex --> bit 0
    // b1-c1-b2-c2 vertex --> bit 1
    // a2-b2-a3-b3 vertex --> bit 8
    // h8-i8-h9-i9 vertex --> bit 63
    uint64_t hbars;
    uint64_t vbars;

    // a1 --> 0, b1 --> 1, a2 --> 16, i9 --> 136
    uint8_t p1loc;
    uint8_t p2loc;

    int p1bars;
    int p2bars;

    bool p1turn;

    
    state(void);
    inline static bool vloc(uint8_t loc);
    inline bool north(uint8_t loc);
    inline bool east(uint8_t loc);
    inline bool south(uint8_t loc);
    inline bool west(uint8_t loc);
    int min_dist(bool p1);
    vector<struct state> next_states(void);
    int cost(void);
};

// Constructor
state::state(void) : hbars(0), vbars(0), p1loc(4), p2loc(132), p1turn(true), p1bars(10), p2bars(10) {}


// Returns whether a square is on the board
inline bool state::vloc(uint8_t loc) {
    return ((loc & 0b1111) <= 9) && (loc <= 136);
}

// Returns whether a square's northern edge can be crossed
inline bool state::north(uint8_t loc) {
    if (!vloc(loc + 16)) return false;
    int row = loc / 16, col = loc % 16;
    if (col == 0) return (this->hbars & ((uint64_t) 1 << (8 * row))) ? false : true;
    if (col == 8) return (this->hbars & ((uint64_t) 1 << (8 * row + 7))) ? false : true;
    return (this->hbars & ((uint64_t) 0b11 << (8 * row + col - 1))) ? false : true;
}


// Returns whether a square's southern edge can be crossed
inline bool state::south(uint8_t loc) {
    if (!vloc(loc - 16)) return false;
    int row = loc / 16, col = loc % 16;
    if (col == 0) return (this->hbars & ((uint64_t) 1 << (8 * (row - 1)))) ? false : true;
    if (col == 8) return (this->hbars & ((uint64_t) 1 << (8 * (row - 1) + 7))) ? false : true;
    return (this->hbars & ((uint64_t) 0b11 << (8 * (row - 1) + col - 1))) ? false : true;
}


// Returns whether a square's eastern edge can be crossed
inline bool state::east(uint8_t loc) {
    if (!vloc(loc + 1)) return false;
    int row = loc / 16, col = loc % 16;
    if (row == 0) return (this->vbars & ((uint64_t) 1 << col)) ? false : true;
    if (row == 8) return (this->vbars & ((uint64_t) 1 << (8 * 7 + col))) ? false : true;
    return (this->vbars & ((uint64_t) 0b100000001 << (8 * (row - 1) + col))) ? false : true;
}


// Returns whether a square's western edge can be crossed
inline bool state::west(uint8_t loc) {
    if (!vloc(loc - 1)) return false;
    int row = loc / 16, col = loc % 16;
    if (row == 0) return (this->vbars & ((uint64_t) 1 << (col - 1))) ? false : true;
    if (row == 8) return (this->vbars & ((uint64_t) 1 << (8 * 7 + col - 1))) ? false : true;
    return (this->vbars & ((uint64_t) 0b100000001 << (8 * (row - 1) + col - 1))) ? false : true;
}


// Returns the min-step path from a player to its finish line, and -1 if there are none
int state::min_dist(bool p1) {
    if ((p1 && p1loc >= 128) || (!p1 && p2loc < 9)) return 0;
    vector<int> pi(144, -1);
    queue<uint8_t> q;
    if (p1) {
        pi.at(p1loc) = -2;
        q.push(p1loc);
        while (!q.empty()) {
            uint8_t loc = q.front();
            q.pop();
            int row = loc / 16, col = loc % 16;
            if (north(loc) && pi.at(loc + 16) == -1) {
                if (loc + 16 >= 128) {
                    // Found a path to rank 9, count number of steps
                    int count = 1;
                    for (int curr = loc; curr != p1loc; curr = pi.at(curr)) count++;
                    return count;
                }
                pi.at(loc + 16) = loc;
                q.push(loc + 16);
            }
            if (east(loc) && pi.at(loc + 1) == -1) {
                pi.at(loc + 1) = loc;
                q.push(loc + 1);
            }
            if (south(loc) && pi.at(loc - 16) == -1) {
                pi.at(loc - 16) = loc;
                q.push(loc - 16);
            }
            if (west(loc) && pi.at(loc - 1) == -1) {
                pi.at(loc - 1) = loc;
                q.push(loc - 1);
            }
        }
        // No path to rank 9 exists, therefore illegal position
        return -1;
    } else {
        pi.at(p2loc) = -2;
        q.push(p2loc);
        while (!q.empty()) {
            uint8_t loc = q.front();
            q.pop();
            int row = loc / 16, col = loc % 16;
            if (south(loc) && pi.at(loc - 16) == -1) {
                // Found a path to rank 1, count number of steps
                if (loc - 16 < 9) {
                    int count = 1;
                    for (int curr = loc; curr != p2loc; curr = pi.at(curr)) count++;
                    return count;
                }
                pi.at(loc - 16) = loc;
                q.push(loc - 16);
            }
            if (west(loc) && pi.at(loc - 1) == -1) {
                pi.at(loc - 1) = loc;
                q.push(loc - 1);
            }
            if (north(loc) && pi.at(loc + 16) == -1) {
                pi.at(loc + 16) = loc;
                q.push(loc + 16);
            }
            if (east(loc) && pi.at(loc + 1) == -1) {
                pi.at(loc + 1) = loc;
                q.push(loc + 1);
            }
        }
        // No path to rank 1 exists, therefore illegal position
        return -1;
    }

}

vector<struct state> state::next_states(void) {
    vector<struct state> res;
    struct state n;
    if (p1turn) {
        if (north(p1loc)) {
            if (p2loc == p1loc + 16) {
                if (north(p1loc + 16)) {
                    n = *this;
                    n.p1loc += 32;
                    n.p1turn = false;
                    res.push_back(n);
                } else {
                    if (east(p1loc + 16)) {
                        n = *this;
                        n.p1loc += 17;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                    if (west(p1loc + 16)) {
                        n = *this;
                        n.p1loc += 15;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p1loc += 16;
                n.p1turn = false;
                res.push_back(n);
            }
        }
        if (east(p1loc)) {
            if (p2loc == p1loc + 1) {
                if (east(p1loc + 1)) {
                    n = *this;
                    n.p1loc += 2;
                    n.p1turn = false;
                    res.push_back(n);
                } else {
                    if (south(p1loc + 1)) {
                        n = *this;
                        n.p1loc -= 15;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                    if (north(p1loc + 1)) {
                        n = *this;
                        n.p1loc += 17;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p1loc += 1;
                n.p1turn = false;
                res.push_back(n);
            }
        }
        if (south(p1loc)) {
            if (p2loc == p1loc - 16) {
                if (south(p1loc - 16)) {
                    n = *this;
                    n.p1loc -= 32;
                    n.p1turn = false;
                    res.push_back(n);
                } else {
                    if (west(p1loc - 16)) {
                        n = *this;
                        n.p1loc -= 17;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                    if (east(p1loc - 16)) {
                        n = *this;
                        n.p1loc -= 15;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p1loc -= 16;
                n.p1turn = false;
                res.push_back(n);
            }
        }
        if (west(p1loc)) {
            if (p2loc == p1loc - 1) {
                if (west(p1loc - 1)) {
                    n = *this;
                    n.p1loc -= 2;
                    n.p1turn = false;
                    res.push_back(n);
                } else {
                    if (north(p1loc - 1)) {
                        n = *this;
                        n.p1loc += 15;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                    if (south(p1loc - 1)) {
                        n = *this;
                        n.p1loc -= 17;
                        n.p1turn = false;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p1loc -= 1;
                n.p1turn = false;
                res.push_back(n);
            }
        }
    } else {
        if (south(p2loc)) {
            if (p1loc == p2loc - 16) {
                if (south(p2loc - 16)) {
                    n = *this;
                    n.p2loc -= 32;
                    n.p1turn = true;
                    res.push_back(n);
                } else {
                    if (west(p2loc - 16)) {
                        n = *this;
                        n.p2loc -= 17;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                    if (east(p2loc - 16)) {
                        n = *this;
                        n.p2loc -= 15;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p2loc -= 16;
                n.p1turn = true;
                res.push_back(n);
            }
        }
        if (west(p2loc)) {
            if (p1loc == p2loc - 1) {
                if (west(p2loc - 1)) {
                    n = *this;
                    n.p2loc -= 2;
                    n.p1turn = true;
                    res.push_back(n);
                } else {
                    if (north(p2loc - 1)) {
                        n = *this;
                        n.p2loc += 15;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                    if (south(p2loc - 1)) {
                        n = *this;
                        n.p2loc -= 17;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p2loc -= 1;
                n.p1turn = true;
                res.push_back(n);
            }
        }
        if (north(p2loc)) {
            if (p1loc == p2loc + 16) {
                if (north(p2loc + 16)) {
                    n = *this;
                    n.p2loc += 32;
                    n.p1turn = true;
                    res.push_back(n);
                } else {
                    if (east(p2loc + 16)) {
                        n = *this;
                        n.p2loc += 17;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                    if (west(p2loc + 16)) {
                        n = *this;
                        n.p2loc += 15;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p2loc += 16;
                n.p1turn = true;
                res.push_back(n);
            }
        }
        if (east(p2loc)) {
            if (p1loc == p2loc + 1) {
                if (east(p2loc + 1)) {
                    n = *this;
                    n.p2loc += 2;
                    n.p1turn = true;
                    res.push_back(n);
                } else {
                    if (south(p2loc + 1)) {
                        n = *this;
                        n.p2loc -= 15;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                    if (north(p2loc + 1)) {
                        n = *this;
                        n.p2loc += 17;
                        n.p1turn = true;
                        res.push_back(n);
                    }
                }
            } else {
                n = *this;
                n.p2loc += 1;
                n.p1turn = true;
                res.push_back(n);
            }
        }        
    }

    if ((p1turn && p1bars == 0) || (!p1turn && p2bars == 0)) {
        // Out of barricades
        return res;
    }

    struct state intermediate = *this;

    if (p1turn) {
        intermediate.p1bars--;
        intermediate.p1turn = false;
    } else {
        intermediate.p2bars--;
        intermediate.p1turn = true;
    }

    uint64_t bar_spaces = ~(hbars | vbars), mask = 1;
    for (; mask; mask <<= 1) {
        if (bar_spaces & mask) {
            // Space available for new barricade
            n = intermediate;
            n.hbars |= mask;
            if (n.min_dist(true) != -1 && n.min_dist(false) != -1) res.push_back(n);
            n = intermediate;
            n.vbars |= mask;
            if (n.min_dist(true) != -1 && n.min_dist(false) != -1) res.push_back(n);
        }
    }

    return res;
}


// Computes the cost of the position for the player whose turn it is
int state::cost(void) {
    int opp_dist = min_dist(!p1turn);
    if (opp_dist == 0) return 1000000; // max penalty upon opponent win
    int self_dist = min_dist(p1turn);
    if (self_dist <= 1) return -1000000; // min penalty upon self win
    int self_bars, opp_bars;
    if (p1turn) {
        self_bars = p1bars;
        opp_bars = p2bars;
    } else {
        self_bars = p2bars;
        opp_bars = p1bars;
    }
    return self_dist - opp_dist;
}

int main(void) {
    state s;
    auto start = chrono::high_resolution_clock::now();
    
    int res = s.min_dist(true);
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = end - start;

    cout << s.next_states().size();
}