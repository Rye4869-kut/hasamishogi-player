#pragma once
#include "board.h"

// INF: 勝ち負けを表す特別な値
// 通常の評価値は必ずこれより小さくなるよう設計する（例: 取り駒差100点 × 最大9枚 = 900）
static const int INF = 1'000'000;

// 繰り返しペナルティ
// INF にしている理由: 繰り返しを「負けと同レベルで嫌う」ため
// 他に合法手があれば必ず繰り返しを避ける強さのペナルティ
static const int REPETITION_PENALTY = INF;

// 局面を me 視点で評価して整数スコアを返す
// state_table: 実際の対局で出現した局面の出現回数テーブル
// state_table を渡している理由:
// 実際の対局で出現した局面の記録は main.cpp だけが持っている
// evaluate に渡すことで探索の深い場所からでも繰り返し情報を参照できる
int evaluate(const Board& b, char me, const StateTable& state_table);