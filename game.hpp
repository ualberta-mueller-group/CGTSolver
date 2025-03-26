#ifndef GAME_H
#define GAME_H

#include "board.hpp"

const char P_PSN = 0;
const char L_PSN = 1;
const char R_PSN = -1;
const char N_PSN = 2;
const char U_PSN = 3;    // unknown

class Game
{
public:
    Game();
    Game(Board board);
    Game(Board board, bool b_wins, bool w_wins);

    bool operator==(const Game& rhs) const;
    bool operator!=(const Game& rhs) const;

    bool is_computed() const { return b_computed && w_computed; };
    bool is_computed_zero() const { return is_computed() && is_zero(); };
    bool is_active() const { return active; };

    bool is_zero() const { assert(is_computed()); return b_wins==false && w_wins==false; };
    bool is_positive() const { assert(is_computed()); return b_wins==true && w_wins==false; };
    bool is_negative() const { assert(is_computed()); return b_wins==false && w_wins==true; };
    bool is_next_win() const { assert(is_computed()); return b_wins==true && w_wins==true; };
    
    bool is_reverse(const Game& other) const;
    bool is_inverse(const Game& other) const;   // { return m_board == other.m_inverse0 || m_board == other.m_inverse1; };

    void set_active(bool status) { active = status; };

    Board get_board() const { return m_board; };
    char get_outcome() const;
    void set_outcome(char outcome);

    std::vector<int> emtpy_points() const;
    bool is_legal_point(int point, Color color) const;
    std::vector<int> legal_points(Color color) const;
    bool is_eye(int point, Color color) const;

    void compute();
    std::vector<Game> play(int point, Color color) const;

//private:
    Board m_board;
    bool b_wins, w_wins, b_computed, w_computed, active;
};

inline Game::Game() :
    b_wins(false), w_wins(false), b_computed(false), w_computed(false),
    active(true)
{ }

inline Game::Game(Board board) :
    m_board(board),
    b_wins(false), w_wins(false), b_computed(false), w_computed(false),
    active(true)
{ }

inline Game::Game(Board board, bool b_wins, bool w_wins) :
    m_board(board),
    b_wins(b_wins), w_wins(w_wins), b_computed(true), w_computed(true),
    active(true)
{ }

inline bool Game::is_reverse(const Game& other) const
{
    if (m_board.size != other.m_board.size)
        return false;
    int size = m_board.size;
    
    int i = 0;
    while (i < size) {
        if (m_board[i] != other.m_board[size-i-1])
            return false;
        i++;
    }
    return true;
}

inline bool Game::is_inverse(const Game& other) const
{
    if (m_board.size != other.m_board.size) {
        return false;
    }
    int size = m_board.size;
    
    bool inverse0 = true, inverse1 = true;
    int i = 0;
    while ((inverse0 || inverse1) && i < size) {
        inverse0 &= m_board[i] == opp_color(other.m_board[i]);
        inverse1 &= m_board[i] == opp_color(other.m_board[size-i-1]);
        i++;
    }
    return inverse0 || inverse1;
}

inline bool Game::operator==(const Game& rhs) const
{
    return m_board == rhs.m_board || is_reverse(rhs);
}

inline bool Game::operator!=(const Game& rhs) const
{
    return m_board != rhs.m_board && ! is_reverse(rhs);
}

inline bool operator<(const Game& game1, const Game& game2)
{
    return boardcmp(game1.m_board, game2.m_board) < 0;
}

inline std::ostream& operator<<(std::ostream& os, Game& game)
{
    os << board_to_string(game.get_board());
    return os;
}

#endif
