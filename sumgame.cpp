#include <iostream>
#include <utility>
#include "unistd.h"

#include "sumgame.hpp"
#include "cache.hpp"
#include "zobrist_hash.hpp"

extern Cache cache;
extern ZobristHash hash;

const int START_MARKER = 0;
const int DEACTIVATE_MARKER = 1;
const int ADD_MARKER = 2;

/////////////////////// SumGame ///////////////////////

SumGame::SumGame()
{
    m_subgames.reserve(100);
}

SumGame::SumGame(Game game)
{
    m_subgames.reserve(100);
    m_subgames.push_back(game);
}

SumGame::SumGame(std::vector<Game>& games)
{
    m_subgames.reserve(100);
    for (Game& game : games) {
        assert(game.is_active());
        m_subgames.push_back(game);
    }
}

void SumGame::set_toplay(int color)
{
    m_toplay = color;
}

Game* SumGame::find_inverse(Game* candidate)
{
    assert(candidate->is_active());
    for (auto& g : m_subgames) {
        if (g.is_active() && g.is_inverse(*candidate)) {
            return &g;
        }
    }
    return 0;
}

bool SumGame::find_inactive(Game* candidate)
{
    for (auto& g : m_subgames) {
        if (&g == candidate) {
            assert(! g.is_active());
            return true;
        }
    }
    return false;
}

void SumGame::deactivate(Game* g)
{
    assert(g->is_active());
    g->set_active(false);
    m_record.push_back(std::make_pair(DEACTIVATE_MARKER, g));
}

void SumGame::add(Game g)
{
    g.m_board = ordered_symmetry(g.m_board);
    m_subgames.push_back(g);
    assert(g.is_active());
    m_record.push_back(std::make_pair(ADD_MARKER, &m_subgames.back()));
    assert(m_subgames.size() <= 100);
}

void SumGame::play(Game& g, int point, bool equivalent_replace)
{
    std::vector<Game> candidates = g.play(point, m_toplay);
    m_record.push_back(std::make_pair(START_MARKER, nullptr));
    deactivate(&g);

    if (candidates.size()==2 && candidates[0].is_inverse(candidates[1])) {
        return;
    }

    for (auto& candidate : candidates) {
        cache.lookup(candidate, equivalent_replace);
        if (!candidate.is_computed_zero()) {
            Game* inverse = find_inverse(&candidate);
            if (inverse) {
                deactivate(inverse);
            }
            else {
                assert(candidate.is_active());
                add(candidate);
            }
        }
    }
}

void SumGame::undo()
{
    assert(! m_record.empty());
    for (;;) {
        auto p = m_record.back();
        m_record.pop_back();
        if (p.first == START_MARKER) {
            break;
        }
        else if (p.first == DEACTIVATE_MARKER) {
            Game* g = p.second;
            assert(find_inactive(g));
            g->set_active(true);
        }
        else {
            assert(p.first == ADD_MARKER);
            assert(p.second == &(m_subgames.back()));
            m_subgames.pop_back();
        }
    }
}

bool SumGame::static_winner(bool& toplay_win)
{
    bool has_positive = false;
    bool has_negative = false;
    bool has_next_win = false;

    for (auto& g : m_subgames) {
        if (g.is_active()) {
            if (! g.is_computed()) {
                return false;
            }
            assert(! g.is_zero());
            if (g.is_next_win()) {
                if (has_next_win)
                    return false;   // N + N = unknown
                has_next_win = true;
            }
            else if (g.is_positive()) {
                if (has_negative)
                    return false;   // L + R = unknown
                has_positive = true;
            }
            else {
                assert(g.is_negative());
                if (has_positive)
                    return false;   // L + R = unknown
                has_negative = true;
            }
        }
    }
    
    assert(! has_negative || ! has_positive);   // !(has_negative && has_positive)
    if (! (has_negative || has_positive || has_next_win)) {
        toplay_win = false; // P-psn
        return true;
    }
    if (m_toplay == BLACK) {
        if (! has_negative) {
            toplay_win = true;
            return true;
        }
        else if (has_next_win) {
            return false;   // R + N = unknown
        }
        else {
            toplay_win = false; // R-psn only
            return true;
        }
    }
    else {
        if (! has_positive) {
            toplay_win = true;
            return true;
        }
        else if (has_next_win) {
            return false;   // L + N = unknown
        }
        else {
            toplay_win = false; // L-psn only
            return true;
        }
    }
}

bool SumGame::negamax(int depth)
{
    bool toplay_win = false;
    bool found = static_winner(toplay_win);
    if (found)
        return toplay_win;
    
    for (auto& g : m_subgames) {
        if (g.is_active()) {
            
            std::vector<int> legal_points = g.legal_points(m_toplay);
            int size = (int)legal_points.size();
            for (int i = 0; i < size; i++) {
                // int idx = 0;
                int idx = (size-i) / 2;

                play(g, legal_points[idx]);
                m_toplay = opp_color(m_toplay);

                toplay_win = ! negamax(depth+1);

                undo();
                m_toplay = opp_color(m_toplay);

                if (toplay_win)
                    return true;

                legal_points.erase(legal_points.begin()+idx);
            }
        }
    }
    return false;
}

uint64_t SumGame::search_stats()
{
    return hash.size2();
}

/////////////////////// HashGame ///////////////////////

bool HashGame::negamax(uint64_t hashcode, const std::vector<std::pair<Game, int>>& subgames, int depth)
{
    int value = hash.get(hashcode, m_toplay);
    if (value != -1)
        return value;
    
    bool toplay_win = false;
    bool found = static_winner(toplay_win);
    if (found) {
        hash.insert(hashcode, toplay_win, m_toplay);
        return toplay_win;
    }
    
    int subgames_size = (int)subgames.size();
    for (int k = subgames_size-1; k >= 0; k--) {
        auto& g = subgames[k];
            
        std::vector<int> legal_points = g.first.legal_points(m_toplay);
        int size = (int)legal_points.size();
        for (int i = 0; i < size; i++) {
            int idx = (size-i) / 2;

            play(m_subgames[g.second], legal_points[idx]);
            m_toplay = opp_color(m_toplay);
            std::vector<std::pair<Game, int>> next_subgames = sort_active_games(m_subgames);
            uint64_t next_hashcode = hash_func(hash, next_subgames);

            toplay_win = ! negamax(next_hashcode, next_subgames, depth+1);

            undo();
            m_toplay = opp_color(m_toplay);

            if (toplay_win) {
                hash.insert(hashcode, true, m_toplay);
                return true;
            }

            legal_points.erase(legal_points.begin()+idx);
            }
    }
    hash.insert(hashcode, false, m_toplay);
    return false;
}

// Select and sort active games in subgames; return list of <game, idx>
std::vector<std::pair<Game, int>> sort_active_games(const std::vector<Game>& subgames)
{
    std::vector<std::pair<Game, int>> active_games;
    int i = 0;
    for (const Game& subgame : subgames) {
        if (subgame.is_active()) {
            std::pair<Game, int> p{subgame, i};
            active_games.push_back(p);
        }
        i++;
    }

    std::sort(active_games.begin(), active_games.end());

    return active_games;
}

bool operator<(const std::pair<Game, int>& p1, const std::pair<Game, int>& p2)
{
    return p1.first < p2.first;
}

//////////////////////// HELPER ////////////////////////

void print_search_stats()
{
    
    return;
}
