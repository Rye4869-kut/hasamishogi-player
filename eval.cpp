#include "eval.h"

// ── 評価関数 ─────────────────────────────────────────
//
// 現在の項目:
//   1. 取り駒差         × 100
//   2. 中央 3×3 支配   ±  5 / 駒
//   3. 前進ボーナス     +  1 / (3行分)
//
// 後でモビリティ・危険度・脅威など追加しやすいように分離
//
int evaluate(const Board& b, char me) {
    char opp = opp_color(me);

    // 1. 取り駒差（ゲームの主要な勝利条件に直結）
    int score = (b.captures[color_idx(me)] - b.captures[color_idx(opp)]) * 100;

    // 2. 中央支配 (rows 3-5, cols 3-5)
    for (int r = 3; r <= 5; r++) {
        for (int c = 3; c <= 5; c++) {
            if      (b.cells[r][c] == me)  score += 5;
            else if (b.cells[r][c] == opp) score -= 5;
        }
    }

    // 3. 前進ボーナス: 相手陣地に近いほど+
    for (int r = 0; r < BS; r++) {
        for (int c = 0; c < BS; c++) {
            char p = b.cells[r][c];
            if (p == me) {
                // 黒は下(大きい r)が前進, 白は上(小さい r)が前進
                score += (me == BLACK) ? r / 3 : (BS - 1 - r) / 3;
            }
        }
    }

    return score;
}