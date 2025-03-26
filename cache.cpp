#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <signal.h>
#include <unistd.h>

#include "cache.hpp"
#include "sumgame.hpp"

uint64_t exponents[2*(MAX_NUM_EMPTY+1)+1];

const char outcome_class[5] = { 'W', 'P', 'B', 'N', 'U'};


Cache::Cache()
{
    int total = 0, accum_size = 0;
    for (int i = 1; i < MAX_NUM_EMPTY+1; i++) {
        m_cache_sizes[i] = std::pow((double)3, i+1);
        m_accum_sizes[i] = accum_size;
        total += m_cache_sizes[i];
        accum_size += m_cache_sizes[i];
    }
    m_accum_sizes[MAX_NUM_EMPTY+1] = accum_size;
    assert(m_accum_sizes[MAX_NUM_EMPTY+1] == total);

    m_cache = new DBEntry[total];

    exponents[0] = 1;
    for (int i = 1; i < 2*(MAX_NUM_EMPTY+1)+1; i++) {
        exponents[i] = 3 * exponents[i-1];
    }
}

Cache::~Cache()
{
    delete[] m_cache;
}

void Cache::lookup(Game& g, bool equivalent_replace) const
{
    int hashcode = hash_func(g.get_board());
    if (hashcode != -1) {
        DBEntry entry = m_cache[hashcode];
        if (equivalent_replace && entry.eq_idx != -1) {
            g = Game(m_cache[entry.eq_idx].board);
            g.b_computed = g.w_computed = true;
            g.b_wins = m_cache[entry.eq_idx].b_wins;
            g.w_wins = m_cache[entry.eq_idx].w_wins;
            return;
        }
        g.b_computed = g.w_computed = true;
        g.b_wins = entry.b_wins;
        g.w_wins = entry.w_wins;
    }
    return;
}

void Cache::compute_outcome(bool store, bool verbose)
{
    for (int i = 1; i < MAX_NUM_EMPTY+1; i++) {
        std::cout << "\r******* NUM_EMPYT " << i << " *******\n";
        int total = m_cache_sizes[i];

        std::vector<Board> boards = construct_boards(i, total);
        assert(total == (int)boards.size());
        for (int j = 0; j < total; j++) {
            int hashcode = m_accum_sizes[i] + j;
            m_cache[hashcode].board = boards[j];
            assert(m_cache[hashcode].eq_idx == -1);
            Game game = Game(boards[j]);
            game.compute();
            set_outcome(m_cache[hashcode], game.get_outcome());
            if (verbose) {
                int outcome = game.get_outcome();
                std::cout << game << "\t" << outcome_class[outcome+1] << "\n";
            }
        }

        std::printf("\33[2K\r");

        if (store)
            store_line(m_cache+m_accum_sizes[i], total, "./db/"+std::to_string(i)+".db");
    }
}

int Cache::hash_func(Board board) const
{
    int num_empty = 0, hashcode = 0;
    Color p = EMPTY;
    for (Point& point : board) {
        num_empty += point == EMPTY;
        if (point == EMPTY && p == EMPTY) {
            hashcode = hashcode * 3 + 0;
        }
        if (point == BLACK) {
            hashcode = hashcode * 3 + 1;
        }
        if (point == WHITE) {
            hashcode = hashcode * 3 + 2;
        }
        p = point;
    }
    if (board[board.size-1] == EMPTY) {
        hashcode *= 3;
    }
    
    if (num_empty > MAX_NUM_EMPTY)
        return -1;
    hashcode += m_accum_sizes[num_empty];
    return hashcode;
}

struct DBFileEntry
{
    char outcome;
    int eq_idx;
} __attribute__((__packed__));


void Cache::store_outcomes()
{
    std::string file_name = "./db/";
    for (int n = 1; n < MAX_NUM_EMPTY+1; n++) {
        store_line(m_cache+m_accum_sizes[n], m_cache_sizes[n], file_name+std::to_string(n)+".db");
    }
}

void Cache::load_outcomes(int up_to_num_empty)
{
    std::cerr << "loading cache...";
    std::string file_name = "./db/";
    for (int n = 1; n < up_to_num_empty+1; n++) {
        std::ifstream f;
        f.open(file_name+std::to_string(n)+".db", std::ios::binary);
        if (f) {
            DBFileEntry* outcomes = (DBFileEntry*) std::malloc(m_cache_sizes[n]*sizeof(DBFileEntry));
            f.read((char*)outcomes, m_cache_sizes[n]*sizeof(DBFileEntry));
            f.close();

            std::vector<Board> list = construct_boards(n, m_cache_sizes[n]);
            for (int i = 0; i < m_cache_sizes[n]; i++) {
                int hashcode = m_accum_sizes[n] + i;
                m_cache[hashcode].board = list[i];
                if (outcomes[i].outcome == N_PSN)
                    m_cache[hashcode].b_wins = m_cache[hashcode].w_wins = true;
                else if (outcomes[i].outcome == L_PSN)
                    m_cache[hashcode].b_wins = true;
                else if (outcomes[i].outcome == R_PSN)
                    m_cache[hashcode].w_wins = true;
                m_cache[hashcode].eq_idx = outcomes[i].eq_idx;
            }

            std::free(outcomes);
        }
    }
    std::cerr << "complete\n";
}

/*******************************************************/
/********************* functions ***********************/
/*******************************************************/

void set_outcome(DBEntry& entry, char outcome)
{
    if (outcome == N_PSN)
        entry.b_wins = entry.w_wins = true;
    else if (outcome == L_PSN)
        entry.b_wins = true;
    else if (outcome == R_PSN)
        entry.w_wins = true;
}

char get_outcome(const DBEntry& entry)
{
    bool b_wins = entry.b_wins, w_wins = entry.w_wins;
    if (b_wins==true && w_wins==true)
        return N_PSN;
    else if (b_wins==true && w_wins==false)
        return L_PSN;
    else if (b_wins==false && w_wins==true)
        return R_PSN;
    else
        return P_PSN;
}

void store_line(DBEntry* ptr, int size, std::string file_name)
{   
    DBFileEntry* outcomes = (DBFileEntry*) std::malloc(size*sizeof(DBFileEntry));
    for (int i = 0; i < size; i++) {
        outcomes[i].outcome = get_outcome(ptr[i]);
        outcomes[i].eq_idx = ptr[i].eq_idx;
    }

    std::ofstream f;
    f.open(file_name, std::ios::binary);
    f.write((char*)outcomes, size*sizeof(DBFileEntry));
    f.close(); 
    std::free(outcomes);
}

const Color placeholder = 3;

std::vector<Board> construct_boards(int num_empty, int total)
{
    Board temp;
    for (int i = 0; i < num_empty; i++) {
        temp.push_back(placeholder);
        temp.push_back(EMPTY);
    }
    temp.push_back(placeholder);
    int size = temp.size;
    assert(size == num_empty*2+1);

    std::vector<Board> list;
    list.reserve(total);
    list.push_back(remove_placeholder(temp));
    for (int i = 1; i < total; i++) {
        for (int j = size-1; j >=0; j-=2) {
            if (temp[j] == placeholder) {
                temp[j] = BLACK;
                break;
            }
            else if (temp[j] == BLACK) {
                temp[j] = WHITE;
                break;
            }
            else {
                temp[j] = placeholder;
            }
        }
        list.push_back(remove_placeholder(temp));
    }
    return list;
}

Board remove_placeholder(Board& board)
{
    Board cboard;
    for (Point& point: board) {
        if (point != placeholder) {
            cboard.push_back(point);
        }
    }
    return cboard;
}

/*******************************************************/
/******************** experiments **********************/
/*******************************************************/

void exp_check_incentive(const Cache& cache, int num_empty)
{
    Game one(cache[12].board, cache[12].b_wins, cache[12].w_wins);    // .x. = +1

    int total = cache.m_accum_sizes[num_empty+1];
    int n = 1;
    for (int i = 0; i < total; i++) {
        if (i == cache.m_accum_sizes[n]) {
            std::cerr << "******* NUM_EMPYT " << n << " *******\n";
            n++;
        }
        Game g(cache[i].board, cache[i].b_wins, cache[i].w_wins);
        std::vector<int> legal_points = g.legal_points(BLACK);
        int size = (int)legal_points.size();
        if (size == 0)
            continue;
        
        for (int point : legal_points) {
            if (g.is_eye(point, BLACK))
                continue;
            
            std::vector<Game> list;
            std::vector<Game> subgames = g.play(point, BLACK);
            for (Game& subgame : subgames) {
                cache.lookup(subgame);
                if (!subgame.is_zero())
                    list.push_back(subgame);
            }
            if (!g.is_zero()) {
                int hashcode = cache.hash_func(inverse_board(g.m_board));
                Game inv_g(cache[hashcode].board, cache[hashcode].b_wins, cache[hashcode].w_wins);
                list.push_back(inv_g);
            }
            list.push_back(one);  // .x. = +1
            SumGame sumgame(list);
            // g' - g + 1 < 0
            // g(list) needs to be an R-psn
            sumgame.set_toplay(BLACK);
            if (sumgame.negamax())
                continue;
            sumgame.set_toplay(WHITE);
            if (!sumgame.negamax())
                continue;
            std::cout << "example game: " << g << " " << point << "\n";
            return;
        }
    }
}
