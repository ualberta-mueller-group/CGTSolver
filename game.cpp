#include <iostream>
#include <cassert>

#include "game.hpp"
#include "sumgame.hpp"

std::vector<int> Game::emtpy_points() const
{
    std::vector<int> points;
    int size = m_board.size;
    for (int i = 0; i < size; i++) {
        if (m_board[i] == EMPTY) {
            points.push_back(i);
        }
    }
    return points;
}

bool Game::is_legal_point(int point, Color color) const
{
    int opp = opp_color(color);
    assert(m_board.size > 0);
    int bound = m_board.size - 1;

    bool empty = m_board[point] == EMPTY;
    bool l = point-1 == 0;;
    bool ll = point-2 >= 0;
    bool r = point+1 == bound;
    bool rr=point+2 <= bound;

    bool l2r2_legal = empty && (ll && rr) && !(m_board[point-1] == m_board[point+1] && m_board[point-1] == opp);
    bool l1r2_legal = empty && (l && rr) && (m_board[0] != opp) && !(m_board[0] == color && m_board[2] == opp);
    bool l2r1_legal = empty && (r && ll) && (m_board[bound] != opp) && !(m_board[bound] == color && m_board[bound-2] == opp);
    bool l1r1_legal = empty && (l && r) && (m_board[0] == EMPTY || m_board[2] == EMPTY) && !(m_board[0] == opp || m_board[2] == opp);
    bool l0_legal = empty && (point == 0 && point<bound) && (m_board[1] == EMPTY || (rr && m_board[1] != opp && m_board[2] == EMPTY));
    bool r0_legal = empty && (point == bound && point>0) && (m_board[bound-1] == EMPTY || (ll && m_board[bound-1] != opp && m_board[bound-2] == EMPTY));


    return l2r2_legal || l1r2_legal || l2r1_legal || l1r1_legal || l0_legal || r0_legal;
}

std::vector<int> Game::legal_points(Color color) const
{
    std::vector<int> points;
    int size = m_board.size;
    for (int i = 0; i < size; i++) {
        bool legal = is_legal_point(i, color);
        if (legal) {
            points.push_back(i);
        }
    }
    return points;
}

bool Game::is_eye(int point, Color color) const
{
    assert(m_board[point]==EMPTY);
    int size = (int)m_board.size;
    if (point==0 && size > 1)
        return m_board[1]==color;
    else if (point==size-1 && size > 1)
        return m_board[size-2]==color;
    else if (size > 2)
        return m_board[point-1]==color && m_board[point+1]==color;
    else
        return false;
}

void Game::compute()
{
    assert(! b_computed);
    assert(! w_computed);
    SumGame sumgame(*this);
    sumgame.set_toplay(BLACK);
    b_wins = sumgame.negamax();
    sumgame.set_toplay(WHITE);
    w_wins = sumgame.negamax();
    b_computed = w_computed = true;
}

std::vector<Game> Game::play(int point, Color color) const
{
    assert(is_active());

    Board cboard = m_board;
    cboard[point] = color;
    cboard = simplify_board(cboard);
    std::vector<Board> subboards = split_board(cboard);

    std::vector<Game> subgames;
    for (auto& subboard : subboards) {
        Game subgame(subboard);
        subgames.push_back(subgame);
    }
    return subgames;
}

char Game::get_outcome() const
{
    if (! b_computed || ! w_computed)
        return U_PSN;
    if (b_wins==false && w_wins==false)
        return P_PSN;
    else if (b_wins==true && w_wins==false)
        return L_PSN;
    else if (b_wins==false && w_wins==true)
        return R_PSN;
    else
        return N_PSN;
}

void Game::set_outcome(char outcome)
{
    if (outcome==P_PSN) {
        b_wins = false;
        w_wins = false;
    }
    else if (outcome==L_PSN) {
        b_wins = true;
        w_wins = false;
    }
    else if (outcome==R_PSN) {
        b_wins = false;
        w_wins = true;
    }
    else {
        assert(outcome==N_PSN);
        b_wins = true;
        w_wins = true;
    }
    b_computed = w_computed = true;
}
