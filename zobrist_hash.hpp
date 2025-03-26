#ifndef H_ZOBRIST_HASH
#define H_ZOBRIST_HASH

#include <cassert>
#include <boost/random.hpp>

#include "utils/hash_map.hpp"
#include "game.hpp"

class ZobristHash
{
public:
    uint64_t m_rntable[4][64];

    ZobristHash(int IDX_bits, int CODE_bits, int ENTRY_bytes);
    ~ZobristHash() {};

    void insert(uint64_t hashcode, int value, int color);
    int get(uint64_t hashcode, int color);

    uint64_t size() { return m_size; }
    uint64_t size2() { return m_size2; }

private:
    int IDX_BITS, CODE_BITS, ENTRY_SIZE;
    uint64_t m_capacity;    // # of entries
    HashMap m_pool;

    uint64_t m_size = 0;
    uint64_t m_size2 = 0;
    
    uint64_t IDX_MASK, CODE_MASK;
    Entry VLD_MASK, b_computed, b_win, w_computed, w_win;
};

inline ZobristHash::ZobristHash(int IDX_bits, int CODE_bits, int ENTRY_bytes)
    : IDX_BITS(IDX_bits), CODE_BITS(CODE_bits), ENTRY_SIZE(ENTRY_bytes),
    m_capacity((uint64_t)1<<IDX_BITS), m_pool(HashMap(m_capacity, ENTRY_SIZE))
{
    assert(IDX_BITS+CODE_BITS <= 8*(int)sizeof(uint64_t));
    assert(CODE_BITS+5 <= 8*ENTRY_SIZE);
    IDX_MASK = (uint64_t)-1 >> (8*sizeof(uint64_t) - IDX_BITS) << CODE_BITS;
    if (CODE_BITS == 0)
        CODE_MASK = 0;
    else
        CODE_MASK = (uint64_t)-1 >> (8*sizeof(uint64_t) - CODE_BITS);
    VLD_MASK = (Entry)1 << CODE_BITS;
    b_computed = (Entry)1 << (CODE_BITS+1);
    b_win = (Entry)1 << (CODE_BITS+2);
    w_computed = (Entry)1 << (CODE_BITS+3);
    w_win = (Entry)1 << (CODE_BITS+4);

    boost::mt19937_64 rng(2024);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 64; j++) {
            m_rntable[i][j] = rng();
        }
    }
}

inline void ZobristHash::insert(uint64_t hashcode, int value, int color)
{
    uint64_t idx = (hashcode & IDX_MASK) >> CODE_BITS;
    uint64_t code = hashcode & CODE_MASK;

    Entry entry;
    while (true) {
        entry = m_pool.get(idx);
        if (entry == 0 || (entry & CODE_MASK) == code)
            break;
        idx = (idx + 1) % m_capacity;
    }

    m_size += entry == 0;
    m_size2++;

    entry |= VLD_MASK;
    entry |= code;
    if (color == BLACK) {
        entry |= b_computed;
        if (value != 0)
            entry |= b_win;
    }
    else {
        entry |= w_computed;
        if (value != 0)
            entry |= w_win;
    }
    m_pool.set(idx, entry);
}

inline int ZobristHash::get(uint64_t hashcode, int color)
{
    uint64_t idx = (hashcode & IDX_MASK) >> CODE_BITS;
    uint64_t code = hashcode & CODE_MASK;

    Entry entry;
    while (true) {
        entry = m_pool.get(idx);
        if (entry == 0 || (entry & CODE_MASK) == code)
            break;
        idx = (idx + 1) % m_capacity;
    }

    if (color == BLACK) {
        if ((entry & b_computed) == 0)
            return -1;
        return (entry & b_win) != 0;
    }
    else {
        if ((entry & w_computed) == 0)
            return -1;
        return (entry & w_win) != 0;
    }
}

//////////////////////// HASH_FUNC ////////////////////////

inline uint64_t hash_func(const ZobristHash& hash, const std::vector<std::pair<Game, int>> subgames)
{
    uint64_t hashcode = 0, idx = 0;
    for (const auto& g : subgames) {
        Board board = g.first.m_board;
        for (Point point : board) {
            hashcode ^= hash.m_rntable[point][idx];
            idx++;
        }
        hashcode ^= hash.m_rntable[3][idx];
        idx++;
    }
    
    return hashcode;
}

#endif
