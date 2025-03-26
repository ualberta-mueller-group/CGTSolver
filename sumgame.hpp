#ifndef SUMGAME_H
#define SUMGAME_H

#include "game.hpp"

class SumGame
{
public:
    SumGame();
    SumGame(Game game);
    SumGame(std::vector<Game>& games);

    void set_toplay(int color);

    void add(Game g);
    void play(Game& g, int point, bool equivalent_replace=true);
    void undo();

    bool static_winner(bool& toplay_win);

    bool negamax(int depth=0);

    uint64_t search_stats();

// private:
    Color m_toplay;
    std::vector<Game> m_subgames;
    std::vector<std::pair<int, Game*>> m_record;

    void deactivate(Game* g);

    Game* find_inverse(Game* candidate);
    bool find_inactive(Game* candidate);
};

class HashGame : public SumGame
{
public:
    HashGame() : SumGame() { };
    HashGame(Game game) : SumGame(game) { };
    HashGame(std::vector<Game>& games) : SumGame(games) { };

    bool negamax(uint64_t hashcode, const std::vector<std::pair<Game, int>>& subgames, int depth=0);
};

std::vector<std::pair<Game, int>> sort_active_games(const std::vector<Game>& subgames);

bool operator<(const std::pair<Game, int>& p1, const std::pair<Game, int>& p2);

void negamax_sig_handler(int signum);

/* a helper function to be called in main */
void print_search_stats();

#endif
