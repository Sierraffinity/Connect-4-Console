#include "C4.h"

#define NUM_COLUMNS BOARD_WIDTH
#define NUM_ROWS BOARD_HEIGHT

#define AI_SEARCH_LEVELS 4
#define AI_DIFFICULTY 0

#define u8 uint8_t
#define bool32 uint32_t
typedef u8 Connect4Table[NUM_ROWS][NUM_COLUMNS];
static Connect4Table* sTables[AI_SEARCH_LEVELS + 1] = {};

static bool32 ConnectFourScreen_IsWinningMove(int x, int player, int tableNum);

static void ConnectFourScreen_PlayPiece(int tableNum, int player, int x);

static int ConnectFourScreen_FindNextYPosInColumn(int x, int tableNum);

int CopyBoardToAIBoard(const struct GameBoard& board)
{

}

// alpha < beta
int ConnectFourScreen_ScoreColumn(int player, int turnCount, int tableNum, int alpha, int beta, int* out)
{
    // searching columns from middle-out
    const static u8 columnOrder[NUM_COLUMNS] = { 3, 2, 4, 1, 5, 0, 6 };

    int x, y, max, aiFuzzRange;
    Connect4Table* table = sTables[tableNum];

    // Cannot calculate without a valid output param
    if (out == NULL)
    {
        return 0;
    }

    // All spaces filled, tie game
    if (turnCount == (NUM_COLUMNS * NUM_ROWS))
    {
        return 0;
    }

    // Check if current player can win on this turn
    for (x = 0; x < NUM_COLUMNS; x++)
    {
        if ((*table)[0][x] == 0 && ConnectFourScreen_IsWinningMove(x, player, tableNum))
        {
            *out = x;
            return ((NUM_COLUMNS * NUM_ROWS) + 1 - turnCount) / 2;
        }
    }

    // Set upper bound of score to possible remaining move count for this player minus one
    max = ((NUM_COLUMNS * NUM_ROWS) - 1 - turnCount) / 2;
    if (beta > max)
    {
        beta = max; // Clamp beta to maximum possible score
        if (alpha >= beta)
        {
            return beta; // window was above max score, now empty
        }
    }

    // Have we looked as deep as we want to go?
    if (tableNum <= AI_SEARCH_LEVELS)
    {
        // Fuzz range [-max, max] scaled to difficulty level
        /*if (player == AI)
        {
            aiFuzzRange = (max * (100 - AI_DIFFICULTY)) / 100;
        }*/

        // Get best score from all possible moves
        for (x = 0; x < NUM_COLUMNS; x++)
        {
            if ((*table)[0][columnOrder[x]] == 0)
            {
                int score = 0;
                if ((sTables[tableNum] != NULL) && (sTables[tableNum + 1] != NULL))
                {
                    memcpy(sTables[tableNum + 1], sTables[tableNum], sizeof(Connect4Table));
                }

                ConnectFourScreen_PlayPiece(tableNum + 1, player, columnOrder[x]);
                score = -ConnectFourScreen_ScoreColumn(!player, turnCount, tableNum + 1, -beta, -alpha, out);

                // Nerf the AI a bit but assume the player is perfect
                /*if (player == AI && aiFuzzRange > 0)
                {
                    //score = ((AI_DIFFICULTY + ((Random() % 200) * (100 - AI_DIFFICULTY) / 100)) * score) / 100;
                    score += (Random() % (aiFuzzRange * 2)) - aiFuzzRange;
                    if (score > max)
                    {
                        score = max;
                    }
                    else if (score < -max)
                    {
                        score = -max;
                    }
                }*/

                if (score >= beta)
                {
                    *out = columnOrder[x];
                    return score;
                }
                else if (score > alpha)
                {
                    alpha = score;
                }
            }
        }
    }

    return alpha;
}

static bool32 ConnectFourScreen_IsWinningMove(int x, int player, int tableNum)
{
    int count, i, j, y, playerPiece;
    Connect4Table* table = sTables[tableNum];

    playerPiece = player + 2;

    // Is column filled?
    if ((*table)[0][x] != 0)
    {
        return FALSE;
    }

    y = ConnectFourScreen_FindNextYPosInColumn(x, tableNum);
    if (y < 0 || y >= NUM_ROWS)
    {
        return FALSE;
    }

    // Check for horizontal win.
    count = 0;
    for (i = x - 1; i >= 0; i--)
    {
        if ((*table)[y][i] != playerPiece)
        {
            break;
        }
        count++;
    }
    for (i = x + 1; i < NUM_COLUMNS; i++)
    {
        if ((*table)[y][i] != playerPiece)
        {
            break;
        }
        count++;
    }
    if (count >= 3)
        return TRUE;

    // Check for vertical win.
    count = 0;
    for (i = y + 1; i < NUM_ROWS; i++)
    {
        if ((*table)[i][x] != playerPiece)
        {
            break;
        }
        count++;
    }
    if (count >= 3)
        return TRUE;

    // Check for / diagonal win.
    count = 0;
    for (i = x - 1, j = y - 1; i >= 0 && j >= 0; i--, j--)
    {
        if ((*table)[j][i] != playerPiece)
        {
            break;
        }
        count++;
    }
    for (i = x + 1, j = y + 1; i < NUM_COLUMNS && j < NUM_ROWS; i++, j++)
    {
        if ((*table)[j][i] != playerPiece)
        {
            break;
        }
        count++;
    }
    if (count >= 3)
        return TRUE;

    // Check for \ diagonal win.
    count = 0;
    for (i = x - 1, j = y + 1; i >= 0 && j < NUM_ROWS; i--, j++)
    {
        if ((*table)[j][i] != playerPiece)
        {
            break;
        }
        count++;
    }
    for (i = x + 1, j = y - 1; i < NUM_COLUMNS && j >= 0; i++, j--)
    {
        if ((*table)[j][i] != playerPiece)
        {
            break;
        }
        count++;
    }
    if (count >= 3)
        return TRUE;

    return FALSE;
}

static void ConnectFourScreen_PlayPiece(int tableNum, int player, int x)
{
    int y = ConnectFourScreen_FindNextYPosInColumn(x, tableNum);
    if (y >= 0 && y < NUM_ROWS)
    {
        (*sTables[tableNum])[y][x] = player + 2;
    }
}

static int ConnectFourScreen_FindNextYPosInColumn(int x, int tableNum)
{
    Connect4Table* table = sTables[tableNum];
    int y;

    for (y = NUM_ROWS - 1; y >= 0; y--)
    {
        if ((*table)[y][x] == 0)
        {
            break;
        }
    }

    return y;
}
