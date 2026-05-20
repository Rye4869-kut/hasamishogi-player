#pragma once
#include "board.h"

// INF: 勝ち負けを表す特別な値
// 通常の評価値は必ずこれより小さくなるよう設計する（例: 取り駒差100点 × 最大9枚 = 900）
static const int INF = 1'000'000;

// 局面を me 視点で評価して整数スコアを返す
int evaluate(const Board& b, char me);