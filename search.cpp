#include "search.h"
#include <algorithm>
#include <vector>
#include <iostream>

static const int REPETITION_PENALTY = INF;

// ── タイマー ──────────────────────────────────────────
// g_start, g_time_limit をグローバル変数にしている理由:
// negamax の再帰呼び出しに毎回引数として渡すとシグネチャが複雑になるため
static TimePoint g_start;
static double    g_time_limit;

static double elapsed() {
    return std::chrono::duration<double>(Clock::now() - g_start).count();
}

// ── 手のオーダリング ──────────────────────────────────
// 取り駒が期待できる手を先頭に，次に中央近くへの手を優先する
// 軽量なヒューリスティック (apply_move は呼ばない)
static int move_score(const Board& b, const Move& mv, char me) {
    char opp = opp_color(me);
    int score = 0;

    // 各方向: 直後に相手駒があり，逆側に自駒があれば捕獲の見込みあり
    for (int d = 0; d < 4; d++) {
        int r = mv.r2 + DR[d], c = mv.c2 + DC[d];
        int cnt = 0;
        while (in_bounds(r, c) && b.cells[r][c] == opp) {
            cnt++; r += DR[d]; c += DC[d];
        }
        if (cnt > 0 && in_bounds(r, c) && b.cells[r][c] == me)
            score += cnt * 60;
    }

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
static int negamax(Board board, int depth, int alpha, int beta, char me,
                   const StateTable& state_table) {
    // 0.95: 再帰の中での打ち切り．本当に時間切れ直前でそれ以上再帰しても意味がない
    if (elapsed() > g_time_limit * 0.95)
        return evaluate(board, me);

    char winner = board.is_game_over();
    if (winner != EMPTY)
        return (winner == me) ? INF : -INF;

    if (depth == 0)
        return evaluate(board, me);

    char opp   = opp_color(me);
    auto moves = order_moves(board, board.generate_legal_moves(me), me);

    if (moves.empty())
        return evaluate(board, me);

    int best = -INF;
    for (const Move& mv : moves) {
        Board next = board;
        next.apply_move(mv.r1, mv.c1, mv.r2, mv.c2, me);

        // 繰り返し検出: 符号反転前に自分視点で直接ペナルティを与える
        // evaluate 内で検出すると Negamax の符号反転で報酬になってしまうためここで判定する
        auto it = state_table.find(next.hash());
        if (it != state_table.end() && it->second >= 2) {
            if (-REPETITION_PENALTY > best) best = -REPETITION_PENALTY;
            continue;
        }

        // Negamax の核心: 相手から見たスコアは自分から見たスコアの符号を反転したもの
        // → どちらのターンでも「自分視点の最大化」だけ書けばよい
        int val = -negamax(next, depth - 1, -beta, -alpha, opp, state_table);
        if (val > best) best = val;
        if (val > alpha) alpha = val;
        // alpha >= beta: 自分がすでに beta 以上を保証できる
        // → 相手はこの枝を選ばないので残りの手を読む必要がない（ベータカット）
        if (alpha >= beta) break;
    }
    return best;
}

Move choose_best_move(const Board& board, char me, double time_limit_sec,
                      const StateTable& state_table) {
    g_start      = Clock::now();
    g_time_limit = time_limit_sec;

    char opp   = opp_color(me);
    auto moves = board.generate_legal_moves(me);
    if (moves.empty()) return {0, 0, 0, 0};

    Move best_move = moves[0];

    for (int depth = 1; ; depth++) {
        // 0.8: 次の深さを始めるか判断．余裕を持って打ち切り，中途半端に終わるのを防ぐ
        if (elapsed() > g_time_limit * 0.8) break;

        int  best_val = -INF - 1;
        Move best_at_depth = best_move;
        bool completed = true;

        auto ordered = order_moves(board, moves, me);
        for (const Move& mv : ordered) {
            // 0.9: 各手の探索開始判断．残り時間が少ないなら新しい手の探索を始めない
            if (elapsed() > g_time_limit * 0.9) { completed = false; break; }

            Board next = board;
            next.apply_move(mv.r1, mv.c1, mv.r2, mv.c2, me);

            // 繰り返し検出: 符号反転前に自分視点で直接ペナルティを与える
            auto it = state_table.find(next.hash());
            if (it != state_table.end() && it->second >= 2) {
                if (-REPETITION_PENALTY > best_val) {
                    best_val      = -REPETITION_PENALTY;
                    best_at_depth = mv;
                }
                continue;
            }

            int val = -negamax(next, depth - 1, -INF, INF, opp, state_table);

            if (val > best_val) {
                best_val      = val;
                best_at_depth = mv;
            }
        }

        // completed が false = 時間切れで途中打ち切り → 全手を比較していないので信頼できない
        // 完走した深さの結果だけを採用する
        if (completed) best_move = best_at_depth;
        std::cerr << "depth: " << depth << "\n" << std::flush;  // 深さ確認
    }

    return best_move;
}