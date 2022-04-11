#pragma once

#include <windows.h> /* for HANDLE type, and console functions */
#include <fcntl.h>
#include <io.h>

#include <stdio.h> /* standard input/output */
#include <stdlib.h> /* included for rand */
#include <string.h>
#include <stdint.h>

#define BOARD_WIDTH 7
#define BOARD_HEIGHT 6

#define NUM_COLUMNS BOARD_WIDTH
#define NUM_ROWS BOARD_HEIGHT

namespace PieceTypeNS
{
    enum Enum : int
    {
        Invalid = -1,
        Air = 0,
        Red = 1,
        Yellow = 2
    };
}

typedef PieceTypeNS::Enum PieceType;

struct GameBoard
{
    void resetCursor();

    GameBoard();

    PieceType get_type(int x, int y) const;

    int get_column_height(int row);

    bool column_valid(int pos);

    bool drop_piece(int pos);

    void reset();

    int buffer[7][6];
    bool dirty;
    int cursorPos;
    bool shouldDropRed;
    bool isComplete;
    bool redWins;
    int turnCount;
};
