#pragma once
#include "Board.h"

static const int INF = 1'000'000;

// 局面を me 視点で評価して整数スコアを返す
int evaluate(const Board& b, char me);