#pragma once
#include "board.h"
#include "eval.h"
#include <chrono>

using Clock     = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

// time_limit_sec 秒以内に反復深化 Alpha-Beta で最善手を選ぶ
// state_table: main.cpp が管理する局面の出現回数テーブル
Move choose_best_move(const Board& board, char me, double time_limit_sec,
                      const StateTable& state_table);