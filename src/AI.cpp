#include "C4.h"
#include <stdlib.h>

#define AI_SEARCH_LEVELS 6
#define AI_DIFFICULTY 50

#define u8 uint8_t
#define u16 uint16_t
#define bool32 uint32_t
typedef u8 Connect4Table[NUM_ROWS][NUM_COLUMNS];
static Connect4Table* sTables[AI_SEARCH_LEVELS + 1] = {};

static bool32 ConnectFourScreen_IsWinningMove(int x, int player, int tableNum);
static void ConnectFourScreen_PlayPiece(int tableNum, int player, int x);
static int ConnectFourScreen_FindNextYPosInColumn(int x, int tableNum);
static void CopyBoardToAIBoard(const struct GameBoard& board);
static int ConnectFourScreen_ScoreColumn(int player, int turnCount, int tableNum, int alpha, int beta);

u16 Random(void)
{
    u16 random = rand() & 0xFFFF;

    char buffer[256] = {};
    sprintf(buffer, "Random = %d\n", random);
    OutputDebugString(buffer);
    return random;
}

bool AIDoMove(struct GameBoard& board)
{
    size_t tablesSize = sizeof(sTables) / sizeof(*sTables);
    Connect4Table* tables = (Connect4Table *)calloc(tablesSize, sizeof(Connect4Table));

    for (int i = 0; i < tablesSize; i++)
    {
        sTables[i] = &tables[i];
    }

    CopyBoardToAIBoard(board);
    int tTurn = 1; // AI turn
    int tTurnCount = board.turnCount;
    int x = ConnectFourScreen_ScoreColumn(tTurn, tTurnCount, 0, -((NUM_COLUMNS * NUM_ROWS) / 2), ((NUM_COLUMNS * NUM_ROWS) / 2));
    free(tables);
    if (!board.column_valid(x))
    {
        char buffer[256] = {};
        sprintf(buffer, "AI tried to play in column %d\n", x);
        OutputDebugString(buffer);
        throw;
    }
    return board.drop_piece(x);
}

void CopyBoardToAIBoard(const struct GameBoard& board)
{
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            int piece = board.buffer[x][BOARD_HEIGHT - 1 - y];
            (*sTables[0])[y][x] = (piece > 0) ? piece + 1 : piece;
        }
    }
}

// alpha < beta
int ConnectFourScreen_ScoreColumn(int player, int turnCount, int tableNum, int alpha, int beta)
{
    // searching columns from middle-out
    const static u8 columnOrder[NUM_COLUMNS] = { 3, 2, 4, 1, 5, 0, 6 };
    int columnScores[NUM_COLUMNS] = { INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN };

    int x, i, max, score;
    Connect4Table* table = sTables[tableNum];

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
            if (tableNum == 0)
            {
                return x;
            }
            else
            {
                return ((NUM_COLUMNS * NUM_ROWS) + 1 - turnCount) / 2;
            }
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
    if (tableNum < AI_SEARCH_LEVELS)
    {
        // Fuzz range [-max, max] scaled to difficulty level
        /*if (player == AI)
        {
            aiFuzzRange = (max * (100 - AI_DIFFICULTY)) / 100;
        }*/

        // Get best score from all possible moves
        for (x = 0; x < NUM_COLUMNS; x++)
        {
            if ((*table)[0][columnOrder[x]] != 0)
            {
                columnScores[columnOrder[x]] = INT_MIN;
            }
            else
            {
                score = 0;
                if ((sTables[tableNum] != NULL) && (sTables[tableNum + 1] != NULL))
                {
                    memcpy(sTables[tableNum + 1], sTables[tableNum], sizeof(Connect4Table));
                }

                ConnectFourScreen_PlayPiece(tableNum + 1, player, columnOrder[x]);
                score = -ConnectFourScreen_ScoreColumn(!player, turnCount + 1, tableNum + 1, -beta, -alpha);

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

                columnScores[columnOrder[x]] = score;

                if (score >= beta)
                {
                    if (tableNum == 0)
                    {
                        return columnOrder[x];
                    }
                    else
                    {
                        return score;
                    }
                }
                else if (score > alpha)
                {
                    alpha = score;
                }
            }
        }
    }

    if (tableNum == 0)
    {
        int shifter = 0;
        max = INT_MIN;

        for (i = 0; i < NUM_COLUMNS; i++)
        {
            if (columnScores[i] > max)
            {
                max = columnScores[i];
            }
        }

        if (AI_DIFFICULTY < 100)
        {
            char buffer[256] = {};
            sprintf(buffer, "Old max: %d\n", max);
            OutputDebugString(buffer);

            // TODO: Fuzz the max
            int absMax = (max < 0) ? -max : max;
            max -= Random() % (absMax * (100 - AI_DIFFICULTY) / 100);
        }

        if (AI_DIFFICULTY < 70)
        {
            shifter = Random() % (7 - (AI_DIFFICULTY / 20));
        }

        for (i = 0; i < NUM_COLUMNS; i++)
        {
            int shifted = (i + shifter) % 7;
            if (columnScores[columnOrder[shifted]] >= max)
            {
                alpha = columnOrder[shifted];
                break;
            }
        }

        char buffer[256] = {};
        sprintf(buffer, "%d %d %d %d %d %d %d\nMax: %d Shifter: %d Out: %d\n", columnScores[0], columnScores[1], columnScores[2], columnScores[3], columnScores[4], columnScores[5], columnScores[6], max, shifter, alpha);
        OutputDebugString(buffer);
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
