#pragma once

bool AIDoMove(struct GameBoard& board);
int ConnectFourScreen_ScoreColumn(int player, int turnCount, int tableNum, int alpha, int beta, int* out);
