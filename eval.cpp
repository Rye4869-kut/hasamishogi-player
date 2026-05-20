#include "eval.h"

int evaluate(const Board& b, char me, const StateTable& state_table) {
    char opp = opp_color(me);

    // 1. 取り駒差（ゲームの主要な勝利条件に直結）
    // 100を掛けている理由: 他の評価項目より圧倒的に重要なためスケールを大きく取る
    // 取り駒1枚差 = 中央支配20マス分 という優先順位を数値で表現している
    int score = (b.captures[color_idx(me)] - b.captures[color_idx(opp)]) * 100;

    // 2. 中央支配 (rows 3-5, cols 3-5)
    for (int r = 3; r <= 5; r++) {
        for (int c = 3; c <= 5; c++) {
            if      (b.cells[r][c] == me)  score += 5;
            else if (b.cells[r][c] == opp) score -= 5;
        }
    }

    // 3. pending_leader の状態を評価
    if (b.pending_leader == me)  score += 500;
    if (b.pending_leader == opp) score -= 500;

    // 4. 繰り返し検出: 3回以上出現した局面にペナルティ
    // state_table.find() が end() を返す場合: その局面がまだ1度も出現していない
    // it->second >= 2 の意味: すでに2回記録済み = これを選ぶと3回目になる
    auto it = state_table.find(b.hash());
    if (it != state_table.end() && it->second >= 2) {
        // 2回記録済み → この手を指すと3回目になる → ペナルティ
        score -= REPETITION_PENALTY;
    }

    return score;
}