#include <vector>
#include <random>
#include <chrono>
#include <cstdint>
#include <iostream>

using namespace std;

static constexpr uint16_t OUT_OF_BOUNDS = 0b1111111000000000;

static constexpr uint16_t FULL_BOARD = 0b0000000111111111;

static constexpr uint16_t COL_1 = 0b0000000100100100;
static constexpr uint16_t COL_2 = 0b0000000010010010;
static constexpr uint16_t COL_3 = 0b0000000001001001;

static constexpr uint16_t ROW_1 = 0b0000000111000000;
static constexpr uint16_t ROW_2 = 0b0000000000111000;
static constexpr uint16_t ROW_3 = 0b0000000000000111;

static constexpr uint16_t DIAG_UP = 0b0000000001010100;
static constexpr uint16_t DIAG_DOWN = 0b0000000100010001;


// this function checks individual player win positions and converts it to numerical values
constexpr int evaluate(uint16_t player, uint16_t agent) {
    if (((player & COL_1) == COL_1) || ((player & COL_2) == COL_2) || ((player & COL_3) == COL_3) ||
        ((player & ROW_1) == ROW_1) || ((player & ROW_2) == ROW_2) || ((player & ROW_3) == ROW_3) ||
        ((player & DIAG_UP) == DIAG_UP) || ((player & DIAG_DOWN) == DIAG_DOWN)) {
        return -10;
    }
    else if (((agent & COL_1) == COL_1) || ((agent & COL_2) == COL_2) || ((agent & COL_3) == COL_3) ||
             ((agent & ROW_1) == ROW_1) || ((agent & ROW_2) == ROW_2) || ((agent & ROW_3) == ROW_3) ||
             ((agent & DIAG_UP) == DIAG_UP) || ((agent & DIAG_DOWN) == DIAG_DOWN)) {
        return 10;
    }
    return 0;
}


constexpr bool is_draw(uint16_t board) {
    return (board & FULL_BOARD) == FULL_BOARD;
}


// gets all possible moves a player can put a marker 
vector<uint16_t> possible_moves(uint16_t player, uint16_t agent) {
    // since we are using 16 bits to represent the board, we have to xor the places we can't go 
    uint16_t board = (~(player | agent)) ^ OUT_OF_BOUNDS;
    vector<uint16_t> choices;
    while (board) {
        // fast way of getting index of lsb
        uint16_t idx = __builtin_ctz(board);
        choices.push_back(idx);
        board ^= 0b1 << idx;
    }
    return choices;
}


int minimax(uint16_t player, uint16_t agent, int depth, bool is_maximizing, int alpha, int beta) {
    int score = evaluate(player, agent);

    // agent won 
    if (score == 10) {
        return score - depth;
    }
    // player won 
    else if (score == -10) {
        return score + depth;
    }
    else if (is_draw(player | agent)) {
        return 0;
    }

    vector<uint16_t> choices = possible_moves(player, agent);

    // the agent
    if (is_maximizing) {
        int best = -1000;
        for (uint16_t choice : choices) {
            agent |= 0b1 << choice;
            best = max(best, minimax(player, agent, depth + 1, false, alpha, beta));
            agent ^= 0b1 << choice;

            alpha = max(alpha, best);
            if (beta <= alpha) {
                break;
            }
        }
        return best;
    }
    // the player
    else {
        int best = 1000;
        for (uint16_t choice : choices) {
            player |= 0b1 << choice;
            best = min(best, minimax(player, agent, depth + 1, true, alpha, beta));
            player ^= 0b1 << choice;

            beta = min(beta, best);
            if (beta <= alpha) {
                break;
            }
        }
        return best;
    }
}


// returns index at which to move bit
uint16_t find_best_move(uint16_t player, uint16_t agent) {
    int best_val = -1000;
    uint16_t best_move;

    // indexes of moves
    vector<uint16_t> choices = possible_moves(player, agent);

    for (uint16_t choice : choices) {
        // do the move
        agent |= 0b1 << choice;

        int move_val = minimax(player, agent, 0, false, -1000, 1000);

        // undo the move
        agent ^= 0b1 << choice;

        if (move_val > best_val) {
            best_move = choice;
            best_val = move_val;
        }
    }
    return best_move;
}


void print_board(uint16_t x_board, uint16_t o_board) {
    uint16_t idx;
    for (int i = 2; i >= 0; i--) {
        for (int j = 2; j >= 0; j--) {
            idx = 0b1 << (i * 3 + j);
            if (x_board & idx) {
                cout << "X ";
            }
            else if (o_board & idx) {
                cout << "O ";
            }
            else {
                cout << (i * 3 + j) << " ";
            }
        }
        printf("\n");
    }
}


void play_game(bool human_goes_first) {
    uint16_t player = UINT16_C(0b0);
    uint16_t agent = UINT16_C(0b0);

    uint16_t move;
    uint16_t ai_move;
    
    cout << "Game starting" << endl;

    if (human_goes_first) {
        print_board(player, agent);
    }
    else {
        print_board(agent, player);
    }
    
    // keep track of whose turn it is 
    bool player_turn = human_goes_first;

    chrono::time_point<chrono::steady_clock> start, end;

    while(true) {
        if (player_turn) {
            cout << "Choose an index to play" << endl;
            cin >> move;
            player |= 0b1 << move;
        }
        else {
            
            start = chrono::steady_clock::now();
            ai_move = find_best_move(player, agent);
            end = chrono::steady_clock::now();

            double elapsed_time = double(chrono::duration_cast <chrono::nanoseconds> (end - start).count());

            cout << "Search time nanoseconds: " << elapsed_time << endl;
            cout << fixed << "Search time seconds: " << elapsed_time / 1e9 << endl;
            
            cout << "Agent played at index " << ai_move << endl;
            cout << endl;
            
            agent |= 0b1 << ai_move;
        }
        player_turn = !player_turn;

        // seeing which to give X and O to 
        if (human_goes_first) {
            print_board(player, agent);
        }
        else {
            print_board(agent, player);
        }

        // terminal conditions 
        if (evaluate(player, agent) == -10) {
            cout << "Player wins" << endl;
            break;
        }
        else if (evaluate(player, agent) == 10) {
            cout << "Agent wins" << endl;
            break;
        }
        else if (is_draw(player | agent)) {
            cout << "Game is drawn" << endl;
            break;
        }
    }
}


int main() {
    bool human_goes_first = false;
    play_game(human_goes_first);
    return 0;
}