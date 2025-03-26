#ifndef COLOR_H
#define COLOR_H

#include <cassert>

typedef unsigned char   Color;

const Color EMPTY = 0;
const Color BLACK = 1;
const Color WHITE = 2;

const Color COLOR_CHAR[3] = {'.', 'x', 'o'};

// is black or white
inline bool is_bw(Color color)
{
    return color == BLACK || color == WHITE;
}

// opponent color
inline Color opp_color(Color color)
{
    return (3 - color) % 3;
}

//////////////////////// I/O ////////////////////////

// convert color to char
inline char color_to_char(Color color)
{
    return COLOR_CHAR[color];
}

// convert char to color
inline Color char_to_color(char c)
{
    assert(c == '.' || c == 'x' || c == 'o');

    int color;
    if (c == '.') {
        color = EMPTY;
    }
    else if (c == 'x')
    {
        color = BLACK;
    }
    else {
        assert(c == 'o');
        color = WHITE;
    }
    return color;
}

#endif
