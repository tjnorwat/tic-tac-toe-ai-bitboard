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

static constexpr uint16_t WINNING_PATTERNS[] = {
    COL_1, COL_2, COL_3,
    ROW_1, ROW_2, ROW_3,
    DIAG_UP, DIAG_DOWN
};

// key for transposition table 
uint64_t hash_key = 0;

// this generates random numbers for our board position 
uint64_t marker_keys[2][9];

void init_random_keys() {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dis;

    // looping over players ( X and O )
    for (int i = 0; i < 2; ++i) 
        // looping over board coordinates
        for (int j = 0; j < 9; ++j) 
            marker_keys[i][j] = dis(gen);
}

// i think this is mainly for starting positions
// since the board is empty it should just be 0 
// will need this for snakes 
uint64_t generate_hash_key(uint16_t player, uint16_t agent) {
    uint64_t final_key = 0ULL;

    // player bitboard should be moved to global 
    uint16_t temp_board = player;
    int index;
    
    uint16_t boards[] = {player, agent};

    for (uint16_t temp_board : boards) {
        while (temp_board) {
            
            index = __builtin_ctz(temp_board);
            // hash the piece
            // 0 is player ; 1 is agent ; index is random int coordinate of the marker 
            // TODO : for snakes , need first index to be player/agent 
            final_key ^= marker_keys[0][index];
            // pop lsb 
            temp_board ^= index; 
        }
    }
    return final_key;
}

// 2 ^ n - 1
// for fast 'modulus'
uint16_t const hash_size = 2047;
// uint16_t const hash_size = 32767;
// uint16_t const hash_size = 65535;
// uint32_t const hash_size = 524288;

#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2

struct tt {
    uint64_t hash_key;
    uint16_t depth;
    uint16_t flag;
    int value;
};

tt hash_table[hash_size];

void clear_hash_table() {
    for (int i = 0; i < hash_size; ++i) {
        hash_table[i].hash_key = 0ULL;
        hash_table[i].depth = 0u;
        hash_table[i].flag = 0u;
        hash_table[i].value = 0;
    }
}

tt read_hash_entry() {
    return hash_table[hash_key & hash_size];
}

void write_hash_entry(uint16_t depth, uint16_t hash_flag, int value) {
    // if the hash size is of power 2^n - 1 we can use the and operator instead of modulus
    tt *hash_entry = &hash_table[hash_key & hash_size];

    hash_entry->hash_key = hash_key;
    hash_entry->depth = depth;
    hash_entry->flag = hash_flag;
    hash_entry->value = value;
}

// this function checks individual player win positions and converts it to numerical values
int evaluate(uint16_t player, uint16_t agent, uint16_t depth) {
    for (uint16_t pattern : WINNING_PATTERNS) {
        if ((player & pattern) == pattern) 
            return -10 + depth;
        else if ((agent & pattern) == pattern)
            return 10 - depth;
    }
    return 0;
}

constexpr bool is_draw(uint16_t board) {
    return (board & FULL_BOARD) == FULL_BOARD;
}

// marker is used for the tt, between 0 player and 1 agent 
int negamax(uint16_t player, uint16_t agent, uint16_t depth, int alpha, int beta, bool marker) {
    int alpha_orig = alpha;

    // see if this position is in the transposition table 
    // tt entry = read_hash_entry();
    tt entry = hash_table[hash_key & hash_size];
    if (entry.hash_key == hash_key && entry.depth >= depth) {
        if (entry.flag == hash_flag_exact)
            return entry.value;
        else if (entry.flag == hash_flag_alpha)
            alpha = max(alpha, entry.value);
        else if (entry.flag == hash_flag_beta) 
            beta = min(beta, entry.value);

        if (alpha >= beta)
            return entry.value;
    }

    int score = evaluate(player, agent, depth);

    if (score)
        return score;
    else if (is_draw(player | agent))
        return 0;

    int value = INT32_MIN;
    // go through open positions 
    uint16_t board = (~(player | agent)) ^ OUT_OF_BOUNDS;
    while (board) {
        uint16_t choice = __builtin_ctz(board);
        board ^= 1u << choice;
        agent |= 1u << choice;
        hash_key ^= marker_keys[marker][choice];
        // have to swap the boards 
        value = max(value, -negamax(agent, player, depth + 1, -beta, -alpha, !marker));
        // undo move / hash
        agent ^= 1u << choice;
        hash_key ^= marker_keys[marker][choice];

        alpha = max(alpha, value);
        if (alpha >= beta)
            break;
    }

    // adding position in tt
    entry.value = value;
    if (value <= alpha_orig)
        entry.flag = hash_flag_beta;
    else if (value >= beta)
        entry.flag = hash_flag_alpha;
    else
        entry.flag = hash_flag_exact;

    entry.depth = depth;

    write_hash_entry(entry.depth, entry.flag, entry.value);
    return value;
}

uint16_t find_best_move(uint16_t player, uint16_t agent) {
    int best_val = INT32_MIN;
    uint16_t best_move;

    // board represents the positions where there is an open slot
    uint16_t board = (~(player | agent)) ^ OUT_OF_BOUNDS;
    while (board) {
        // get the index of an open position
        uint16_t choice = __builtin_ctz(board);
        // remove that index 
        board ^= 1u << choice;

        // play the index 
        agent |= 1u << choice;
        hash_key ^= marker_keys[1][choice];

        int move_val = -negamax(agent, player, 0, INT32_MIN, INT32_MAX, false);

        // undo the move / hash
        agent ^= 1u << choice;
        hash_key ^= marker_keys[1][choice];

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
            idx = 1u << (i * 3 + j);
            if (x_board & idx)
                cout << "X ";
            else if (o_board & idx)
                cout << "O ";
            else
                cout << (i * 3 + j) << " ";
        }
        cout << endl;
    }
}

void play_game(bool human_goes_first) {
    uint16_t player = 0u;
    uint16_t agent = 0u;

    uint16_t move;
    uint16_t ai_move;
    
    cout << "Game starting" << endl;

    if (human_goes_first)
        print_board(player, agent);
    else
        print_board(agent, player);
    
    // keep track of whose turn it is 
    bool player_turn = human_goes_first;

    chrono::time_point<chrono::steady_clock> start, end;

    while(true) {
        if (player_turn) {
            cout << "Choose an index to play" << endl;
            cin >> move;
            player |= 1u << move;
            hash_key ^= marker_keys[0][move];
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

            agent |= 1u << ai_move;
            hash_key ^= marker_keys[1][ai_move];
        }
        player_turn = !player_turn;
        
        // seeing which to give X and O to 
        if (human_goes_first)
            print_board(player, agent);
        else
            print_board(agent, player);

        int value = evaluate(player, agent, 0);

        // terminal conditions 
        if (value == -10) {
            cout << "Player wins" << endl;
            break;
        }
        else if (value == 10) {
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
    init_random_keys();
    clear_hash_table();
    // hash_key = generate_hash_key();

    bool human_goes_first = false;
    play_game(human_goes_first);

    return 0;
}