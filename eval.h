#pragma once
#include "board.h"

static const int INF = 1'000'000;

// 繰り返しペナルティ
static const int REPETITION_PENALTY = 300;  // 3回以上出現した局面へのペナルティ

// 局面を me 視点で評価して整数スコアを返す
// state_table: 実際の対局で出現した局面の出現回数テーブル
int evaluate(const Board& b, char me, const StateTable& state_table);