#include "C4.h"
#include "AI.h"

#define WIDTH 70
#define HEIGHT 35

HANDLE wHnd; /* write (output) handle */
HANDLE rHnd; /* read (input handle */

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
    void resetCursor()
    {
        cursorPos = BOARD_WIDTH / 2;
    }

    GameBoard()
    {
        reset();
    }

    PieceType get_type(int x, int y)
    {
        if (x < 0 || y < 0 || x > BOARD_WIDTH || y > BOARD_HEIGHT)
        {
            return PieceType::Invalid;
        }
        return static_cast<PieceType>(buffer[x][y]);
    }

    int get_column_height(int row)
    {
        for (int i = 0; i < 5; ++i)
        {
            if (buffer[row][i] == PieceType::Air)
            {
                return i;
            }
        }
        return 5;
    }

    bool column_valid(int pos)
    {
        return !isComplete && buffer[pos][5] == PieceType::Air;
    }

    bool AIDoMove(void)
    {
        CopyBoardToAIBoard(this->buffer);

    }

    bool drop_piece(int pos)
    {
        if (!column_valid(pos)) return false;

        int newPos = get_column_height(pos);
        auto targetType = shouldDropRed ? PieceType::Red : PieceType::Yellow;
        shouldDropRed = !shouldDropRed;

        buffer[pos][newPos] = targetType;

        for (int dir = 0; dir < 8; ++dir)
        {
            int ox = 0;
            int oy = 0;
            switch (dir)
            {
            // Right
            case 7:
                ox = 1;
                oy = -1;
                break;
            case 0:
                ox = 1;
                oy = 0;
                break;
            case 1:
                ox = 1;
                oy = 1;
                break;
            // Up/down
            case 2:
                ox = 0;
                oy = 1;
                break;
            case 6:
                ox = 0;
                oy = -1;
                break;
            // Left
            case 5:
                ox = -1;
                oy = -1;
                break;
            case 4:
                ox = -1;
                oy = 0;
                break;
            case 3:
                ox = -1;
                oy = 1;
                break;
            }

            int originX = pos;
            int originY = newPos;

            while (get_type(originX - ox, originY - oy) == targetType)
            {
                originX -= ox;
                originY -= oy;
            }

            isComplete = true;
            for (int i = 1; i < 4; ++i)
            {
                int x = originX + ox * i;
                int y = originY + oy * i;
                if (get_type(x, y) != targetType)
                {
                    isComplete = false;
                    break;
                }
            }

            if (isComplete)
            {
                redWins = targetType == PieceType::Red;
                break;
            }

            dirty = true;
        }

        if (!shouldDropRed)
            AIDoMove();

        return true;
    }

    void reset()
    {
        memset(buffer, 0, sizeof(buffer));
        dirty = true;
        resetCursor();
        shouldDropRed = true;
        isComplete = false;
        redWins = false;
    }

    int buffer[7][6];
    bool dirty;
    int cursorPos;
    bool shouldDropRed;
    bool isComplete;
    bool redWins;
};

void render(GameBoard& board, CHAR_INFO* consoleBuffer, COORD& characterBufferSize, COORD& characterPosition, SMALL_RECT& consoleWriteArea)
{
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            consoleBuffer[x + WIDTH * y].Char.AsciiChar = (unsigned char)219;
            consoleBuffer[x + WIDTH * y].Attributes = 0;
        }
    }

    const int xMultiplier = 3;
    const int yMultiplier = 2;

    int dropPos = (WIDTH / 2 - BOARD_WIDTH / 2 * xMultiplier) + board.cursorPos * xMultiplier +     // X
        (HEIGHT / 2 - BOARD_HEIGHT / 2 * yMultiplier) * WIDTH;                                      // Y

    consoleBuffer[dropPos].Attributes = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
    consoleBuffer[dropPos].Char.AsciiChar = 'V';

    for (int bx = 0; bx < BOARD_WIDTH; ++bx)
    {
        int x = (WIDTH / 2 - BOARD_WIDTH / 2 * xMultiplier) + bx * xMultiplier;
        for (int by = 0; by < BOARD_HEIGHT; ++by)
        {
            int y = (BOARD_HEIGHT - by) * yMultiplier + (HEIGHT / 2 - BOARD_HEIGHT / 2 * yMultiplier);
            auto piece = board.get_type(bx, by);
            switch (piece)
            {
            case PieceType::Red:
                consoleBuffer[x + WIDTH * y].Attributes = FOREGROUND_RED;
                consoleBuffer[x + WIDTH * y].Char.UnicodeChar = u'\u25CF';
                break;
            case PieceType::Yellow:
                consoleBuffer[x + WIDTH * y].Attributes = FOREGROUND_RED | FOREGROUND_GREEN;
                consoleBuffer[x + WIDTH * y].Char.UnicodeChar = u'\u25CB';
                break;
            case PieceType::Air:
                consoleBuffer[x + WIDTH * y].Attributes = FOREGROUND_BLUE;
                consoleBuffer[x + WIDTH * y].Char.UnicodeChar = u'\u25CB';
                //consoleBuffer[x + WIDTH * y].Char.AsciiChar = 'A';
                break;
            }
        }
    }
    
    char* nextMoveString = "Red's turn\nUse the arrow keys to move.\nPress \"Enter\" to drop your piece.";
    if (board.isComplete)
    {
        if (board.redWins)
        {
            nextMoveString = "Red wins!  Press R to reset the game.";
        }
        else
        {
            nextMoveString = "Yellow wins!  Press R to reset the game.";
        }
    }
    else if (!board.shouldDropRed)
    {
        nextMoveString = "Yellow's turn\nUse the arrow keys to move.\nPress \"Enter\" to drop your piece.";
    }

    int len = strlen(nextMoveString);
    int xlen = 0;
    {
        int curXLen = xlen;
        for (int i = 0; i < len; ++i)
        {
            if (nextMoveString[i] == '\n')
            {
                if (curXLen > xlen)
                {
                    curXLen = 0;
                    xlen = curXLen;
                }
                continue;
            }
            if (nextMoveString[i] == '\r')
            {
                continue;
            }
            ++curXLen;
        }

        if (curXLen > xlen)
        {
            xlen = curXLen;
        }
    }
    int startOffset = (HEIGHT / 2 - BOARD_HEIGHT + 2 + BOARD_HEIGHT * 2) * WIDTH + WIDTH / 2 - xlen / 2;
    int off = 0;
    for (int i = 0; i < len; ++i)
    {
        if (nextMoveString[i] == '\n')
        {
            startOffset += WIDTH;
            off = 0;
            continue;
        }
        consoleBuffer[startOffset + off].Attributes = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
        consoleBuffer[startOffset + off].Char.AsciiChar = nextMoveString[i];
        ++off;
    }

    /* Write our character buffer (a single character currently) to the console buffer */
    WriteConsoleOutputA(wHnd, consoleBuffer, characterBufferSize, characterPosition, &consoleWriteArea);
}

void update(GameBoard& board)
{
    static bool wasLeft, wasRight, wasEnter, wasR = false;

    bool left = GetAsyncKeyState(VK_LEFT) != 0;
    bool right = GetAsyncKeyState(VK_RIGHT) != 0;
    bool enter = GetAsyncKeyState(VK_RETURN) != 0;
    
    // R from https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    // 0x52 == ascii R http://www.asciitable.com/
    bool rDown = GetAsyncKeyState('R') != 0;

    if (left != wasLeft)
    {
        if (left)
        {
            board.cursorPos--;
            if (board.cursorPos < 0)
                board.cursorPos = 0;
            board.dirty = true;
        }
    }
    wasLeft = left;

    if (right != wasRight)
    {
        if (right)
        {
            board.cursorPos++;
            if (board.cursorPos > BOARD_WIDTH - 1)
                board.cursorPos = BOARD_WIDTH - 1;
            board.dirty = true;
        }
    }
    wasRight = right;

    if (enter && !wasEnter)
    {
        if (enter)
        {
            if (board.drop_piece(board.cursorPos))
            {
                board.resetCursor();
            }
        }
    }
    wasEnter = enter;

    if (rDown != wasR)
    {
        if (rDown && board.isComplete)
        {
            board.reset();
        }
    }
    wasLeft = left;

    // TODO: Game loop
    Sleep(4);
}

int main(void)
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    /* Window size coordinates, be sure to start index at zero! */
    SMALL_RECT windowSize = { 0, 0, WIDTH - 1, HEIGHT - 1 };

    /* A COORD struct for specificying the console's screen buffer dimensions */
    COORD bufferSize = { WIDTH, HEIGHT };

    /* Setting up different variables for passing to WriteConsoleOutput */
    COORD characterBufferSize = { WIDTH, HEIGHT };
    COORD characterPosition = { 0, 0 };
    SMALL_RECT consoleWriteArea = { 0, 0, WIDTH - 1, HEIGHT - 1 };

    /* A CHAR_INFO structure containing data about a single character */
    CHAR_INFO consoleBuffer[WIDTH * HEIGHT];

    /* initialize handles */
    wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
    rHnd = GetStdHandle(STD_INPUT_HANDLE);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(wHnd, &info);

    /* Set the console's title */
    SetConsoleTitle("Connect 4");

    /* Set the window size */
    SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

    /* Set the screen's buffer size */
    SetConsoleScreenBufferSize(wHnd, bufferSize);

    GameBoard board;
    //board.drop_piece(PieceType::Red, 3);
    //board.drop_piece(PieceType::Yellow, 3);
    while (1)
    {
        if (board.dirty)
        {
            render(board, consoleBuffer, characterBufferSize, characterPosition, consoleWriteArea);
        }
        update(board);
    }
}