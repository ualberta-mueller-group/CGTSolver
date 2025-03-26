#ifndef CGTBOARD_H
#define CGTBOARD_H

#include <stdint.h>
#include <vector>
#include <string>
#include <algorithm>

#include "color.hpp"


const int MAX_BOARD_LEN = 64;

typedef Color   Point;


class Board
{
public:
    Point board[MAX_BOARD_LEN];
    uint8_t size = 0;

    Board() { };
    Board(int size);

    void push_back(Point point);
    void clear() { size = 0; }

    Point& operator[](size_t pos) { return board[pos]; }
    Point operator[](size_t pos) const { return board[pos]; }
    bool operator==(const Board& rhs) const;
    bool operator!=(const Board& rhs) const;

    Point* begin() { return board; }
    Point* end() { return board+size; }
};

inline Board::Board(int size)
{
    for (int i = 0; i < size; i++) {
        board[i] = EMPTY;
    }
    this->size = size;
}

inline void Board::push_back(Point point)
{
    assert(size < MAX_BOARD_LEN);
    board[size] = point;
    size++;
    return;
}

// equal in color
inline bool Board::operator==(const Board& rhs) const
{
    if (size != rhs.size)
        return false;
    for (int i = 0; i < size; i++) {
        if (board[i] != rhs[i])
            return false;
    }
    return true;
}

// not equal in color
inline bool Board::operator!=(const Board& rhs) const
{
    return ! operator==(rhs);
}

//////////////////////// FUNCTIONS ////////////////////////

// reverse board
inline Board reverse_board(const Board board)
{
    Board reversed = board;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

// inverse board; G -> -G
inline Board inverse_board(const Board board)
{
    Board inversed = board;
    for (auto& point : inversed) {
        point = opp_color(point);
    }
    return inversed;
}

// simplify board
inline Board simplify_board(const Board board)
{
    Board cboard;
    cboard.push_back(board[0]);
    Color previous = board[0];
    for (int i = 1; i < board.size; i++) {
        Color color = board[i];
        if (color == EMPTY || color != previous) {
            cboard.push_back(board[i]);
        }
        previous = color;
    }
    return cboard;
}

// split board
inline std::vector<Board> split_board(const Board board)
{
    std::vector<Board> subboards;

    int size = (int)board.size;
    Color previous = board[0];
    Board subboard;
    subboard.push_back(board[0]);
    for (int i = 1; i < size; i++) {
        char c = board[i];
        if (c != EMPTY && c == opp_color(previous)) {
            subboards.push_back(subboard);
            subboard.clear();
        }
        subboard.push_back(board[i]);
        previous = c;
    }
    subboards.push_back(subboard);

    return subboards;
}

inline bool validate_board(Board board)
{
    board = simplify_board(board);
    int size = board.size;

    if (size==1 && board[0]!=EMPTY) return false;
    if (size==2 && board[0]!=EMPTY && board[1]!=EMPTY) return false; 
    if (board[0]!=EMPTY && board[1]==opp_color(board[0])) return false;
    if (board[size-1]!=EMPTY && board[size-2]==opp_color(board[size-1])) return false;

    for (int i = 1; i < size-1; i++) {
        char color = board[i];
        if (color!=EMPTY && board[i-1]!=EMPTY && board[i+1]!=EMPTY) {
            return false;
        }
    }
    return true;
}

// compare boards as strings
inline int boardcmp(const Board& board1, const Board& board2)
{
    if (board1.size != board2.size) {
        return board1.size - board2.size;
    }
    else {
        int size = board1.size;
        for (int i = 0; i < size; i++) {
            if (board1[i] != board2[i]) {
                return board1[i] - board2[i];
            }
        }
    }
    return 0;
}

// return the smaller one of the board and its reverse
inline Board ordered_symmetry(const Board& board)
{
    int size = board.size;
    int half = size / 2;
    for (int i = 0; i < half; i++) {
        int diff = board[i] - board[size-1-i];
        if (diff < 0) {
            return board;
        }
        else if (diff > 0) {
            return reverse_board(board);
        }
    }
    return board;
}

//////////////////////// I/O ////////////////////////

// convert board to string
inline std::string board_to_string(Board board)
{
    std::string sboard;
    for (int i = 0; i < board.size; i++) {
        assert(board[i] == EMPTY || board[i] == BLACK || board[i] == WHITE);
        sboard += color_to_char(board[i]);
    }
    return sboard;
}

// convert string to board
inline Board string_to_board(std::string sboard)
{
    int size = sboard.size();
    Board board;
    for (int i = 0; i < size; i++) {
        board.push_back(Point(char_to_color(sboard[i])));
    }
    return board;
}

inline std::ostream& operator<<(std::ostream& os, Board& board)
{
    os << board_to_string(board);
    return os;
}

#endif
