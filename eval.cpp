#include "eval.h"

int evaluate(const Board& b, char me) {
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

    // 4. 脅威評価: 次の手で取られる危険がある駒にペナルティ
    // 取り駒差1枚（100点）より小さい値にすることで
    // 「危険を避けるためなら駒を損する」という判断を防ぐ
    for (int r = 0; r < BS; r++) {
        for (int c = 0; c < BS; c++) {
            if (b.cells[r][c] != me) continue;
            // 4方向について，その軸で両側に相手駒がいれば危険
            for (int d = 0; d < 2; d++) {  // 縦軸・横軸の2軸だけ確認
                int rp = r + DR[d], cp = c + DC[d];  // 正方向
                int rn = r - DR[d], cn = c - DC[d];  // 負方向
                if (!in_bounds(rp, cp) || !in_bounds(rn, cn)) continue;
                if (b.cells[rp][cp] == opp && b.cells[rn][cn] == opp) {
                    score -= 80;  // 取り駒差1枚（100点）より少し小さい値
                }
            }
        }
    }

    return score;
}