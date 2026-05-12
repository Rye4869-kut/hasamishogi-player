#include "search.h"
#include <algorithm>
#include <vector>

// ── タイマー ──────────────────────────────────────────
static TimePoint g_start;
static double    g_time_limit;

static double elapsed() {
    return std::chrono::duration<double>(Clock::now() - g_start).count();
}

// ── 手のオーダリング ──────────────────────────────────
// 取り駒が期待できる手を先頭に、次に中央近くへの手を優先する
// 軽量なヒューリスティック (apply_move は呼ばない)
static int move_score(const Board& b, const Move& mv, char me) {
    char opp = opp_color(me);
    int score = 0;

    // 各方向: 直後に相手駒があり、逆側に自駒があれば捕獲の見込みあり
    for (int d = 0; d < 4; d++) {
        int r = mv.r2 + DR[d], c = mv.c2 + DC[d];
        int cnt = 0;
        while (in_bounds(r, c) && b.cells[r][c] == opp) {
            cnt++; r += DR[d]; c += DC[d];
        }
        if (cnt > 0 && in_bounds(r, c) && b.cells[r][c] == me)
            score += cnt * 60;  // 取れそうな駒数に比例
    }

    // 中央への距離が小さいほど加点
    score -= (abs(4 - mv.r2) + abs(4 - mv.c2));

    return score;
}

static std::vector<Move> order_moves(const Board& b,
                                     std::vector<Move> moves,
                                     char me)
{
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b_mv) {
        return move_score(b, a, me) > move_score(b, b_mv, me);
    });
    return moves;
}

// ── Negamax Alpha-Beta ───────────────────────────────
// Board を値渡し（コピー）することで手の "undo" を不要にする
// 9×9 の char 配列なのでコピーコストは低い
static int negamax(Board board, int depth, int alpha, int beta, char me) {
    // 時間切れ → 葉ノードと同様に評価値を返す
    if (elapsed() > g_time_limit * 0.95)
        return evaluate(board, me);

    // 終局判定
    char winner = board.is_game_over();
    if (winner != EMPTY)
        return (winner == me) ? INF : -INF;

    // 葉ノード
    if (depth == 0)
        return evaluate(board, me);

    char opp   = opp_color(me);
    auto moves = order_moves(board, board.generate_legal_moves(me), me);

    if (moves.empty())
        return evaluate(board, me);  // 合法手なし (基本的に起きないはずだが念のため)

    int best = -INF;
    for (const Move& mv : moves) {
        Board next = board;                           // 盤面コピー
        next.apply_move(mv.r1, mv.c1, mv.r2, mv.c2, me);
        int val = -negamax(next, depth - 1, -beta, -alpha, opp);
        if (val > best) best = val;
        if (val > alpha) alpha = val;
        if (alpha >= beta) break;                     // Beta カット
    }
    return best;
}

// ── 反復深化 ─────────────────────────────────────────
Move choose_best_move(const Board& board, char me, double time_limit_sec) {
    g_start      = Clock::now();
    g_time_limit = time_limit_sec;

    char opp   = opp_color(me);
    auto moves = board.generate_legal_moves(me);
    if (moves.empty()) return {0, 0, 0, 0};  // 合法手なし (安全策)

    Move best_move = moves[0];

    // depth=1 から始めて時間が許す限り深くする
    for (int depth = 1; ; depth++) {
        if (elapsed() > g_time_limit * 0.8) break;

        int  best_val = -INF - 1;
        Move best_at_depth = best_move;   // 前の深さの結果をデフォルトに
        bool completed = true;

        auto ordered = order_moves(board, moves, me);
        for (const Move& mv : ordered) {
            if (elapsed() > g_time_limit * 0.9) { completed = false; break; }

            Board next = board;
            next.apply_move(mv.r1, mv.c1, mv.r2, mv.c2, me);
            int val = -negamax(next, depth - 1, -INF, INF, opp);

            if (val > best_val) {
                best_val       = val;
                best_at_depth  = mv;
            }
        }

        // 完走した深さの結果だけ採用（途中打ち切りは信頼しない）
        if (completed) best_move = best_at_depth;
    }

    return best_move;
}