#include <iostream>
#include <random>
#include <cmath>
#include <fstream>
#include <chrono>
#include <signal.h>
#include <unistd.h>

#include "board.hpp"
#include "cache.hpp"
#include "sumgame.hpp"
#include "zobrist_hash.hpp"

Cache cache;
ZobristHash hash(36, 27, 4);

std::vector<Game> process_inputs(int argc, char** argv);


int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "usage: solver_main [board...] [player]\n\n" <<
                        "    board\tstring of .ox\n" <<
                        "    player\tb or w\n\n" <<
                        "  example: solver_main .x..ox. b\n";
        return 0;
    }
    int toplay = (argv[argc-1][0]=='b') ? BLACK : WHITE;
    
    cache.load_outcomes(MAX_NUM_EMPTY);

    std::vector<Game> games = process_inputs(argc, argv);
    HashGame sumgame(games);
    sumgame.set_toplay(toplay);
    std::vector<std::pair<Game, int>> sorted_games = sort_active_games(sumgame.m_subgames);
    uint64_t hashcode = hash_func(hash, sorted_games);

    //signal(SIGALRM, negamax_sig_handler);
    //alarm(10);

    auto beg = std::chrono::high_resolution_clock::now();
    bool win = sumgame.negamax(hashcode, sorted_games, 1);
    auto end = std::chrono::high_resolution_clock::now();

    //alarm(0);
    //std::fprintf(stderr, "\33[2K\r");

    auto ms_int = std::chrono::duration_cast<std::chrono::seconds>(end - beg);

    std::cout << win << "\t" << ms_int.count() << "s\t" << hash.size2() << " nodes\n";

    return 0;
}


std::vector<Game> process_inputs(int argc, char** argv)
{
    std::vector<Game> tmp_games;
    for (int i = 1; i < argc-1; i++) {
        Board board = simplify_board(string_to_board(argv[i]));
        std::vector<Board> subboards = split_board(board);
        for (Board& subboard : subboards) {
            Game game(subboard);
            cache.lookup(game);
            if (game.is_computed_zero()) {
                game.set_active(false);
            }
            else {
                game.m_board = ordered_symmetry(game.m_board);
                int size = (int)tmp_games.size();
                for (int j = 0; j < size; j++) {
                    if (tmp_games[j].is_active() && game.is_inverse(tmp_games[j])) {
                        tmp_games[j].set_active(false);
                        game.set_active(false);
                        break;
                    }
                }
            }
            tmp_games.push_back(game);
        }
    }

    std::vector<Game> games;
    for (Game& game : tmp_games) {
        if (game.is_active())
            games.push_back(game);
    }
    return games;
}
