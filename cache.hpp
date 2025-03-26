#ifndef CACHE_H
#define CACHE_H

#include <array>
#include <tuple>
#include <fstream>

#include "game.hpp"

const int MAX_NUM_EMPTY = 15;

struct DBEntry
{
    Board board;
    bool b_wins = false;
    bool w_wins = false;
    int eq_idx = -1;    // idx of the simplest equivalent w.r.t. m_head
};


class Cache
{
public:
    Cache();
    ~Cache();

    DBEntry& operator[](size_t pos) { return m_cache[pos]; }
    DBEntry operator[](size_t pos) const { return m_cache[pos]; };
    DBEntry* begin() { return m_cache; };
    DBEntry* end() { return m_cache + m_accum_sizes[MAX_NUM_EMPTY+1]; }

    int hash_func(Board board) const;
    void lookup(Game& g, bool equivalent_replace=true) const;

    void compute_outcome(bool store=false, bool verbose=false);

    void store_outcomes();
    void load_outcomes(int up_to_num_empty);

//private:
    DBEntry* m_cache;
    int m_cache_sizes[MAX_NUM_EMPTY+1];
    int m_accum_sizes[MAX_NUM_EMPTY+2];
};

/*******************************************************/
/********************* functions ***********************/
/*******************************************************/

void set_outcome(DBEntry& ptr, char outcome);

char get_outcome(const DBEntry& ptr);

void store_line(DBEntry* ptr, int size, std::string file_name);

std::vector<Board> construct_boards(int num_empty, int total);

Board remove_placeholder(Board& board);

/*******************************************************/
/******************** experiments **********************/
/*******************************************************/

/* Check if there exists such a bad move that g -> g' where g' < g - 1 */
/* Check if there exists an eye-filling move that g -> g' where g' > g - 1 */
/* Check if there exists a non-eye-filling move that g -> g' where g' <= g - 1 */
void exp_check_incentive(const Cache& cache, int num_empty);


#endif
