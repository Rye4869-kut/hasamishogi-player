#include <iostream>
#include <string>
#include "board.h"
#include "search.h"

static const double TIME_LIMIT = 1.0;  // 1手あたりの思考時間 (秒)
static const char*  PLAYER_NAME = "MyPlayer";  // ← 自分の名前に変える

// 末尾の改行・スペースを除去
static std::string trim(std::string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
        s.pop_back();
    return s;
}

// "r1c1r2c2" 形式の 4 桁文字列を Move に変換
static bool parse_move(const std::string& s, Move& mv) {
    if (s.size() < 4) return false;
    for (int i = 0; i < 4; i++)
        if (s[i] < '0' || s[i] > '8') return false;
    mv.r1 = s[0] - '0';
    mv.c1 = s[1] - '0';
    mv.r2 = s[2] - '0';
    mv.c2 = s[3] - '0';
    return true;
}

int main() {
    // C の printf/scanf との同期を切って入出力を高速化
    // cin.tie(nullptr): cout が cin の前に自動でフラッシュする動作を無効化
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;

    // ── Step 1: "OK?" を受け取ってプレイヤー名を返す ──
    std::getline(std::cin, line);
    std::cout << PLAYER_NAME << "\n" << std::flush;

    // ── Step 2: 色 ("Black" / "White") を受け取る ────
    std::getline(std::cin, line);
    line = trim(line);
    char my_color = (line[0] == 'B') ? BLACK : WHITE;
    char opp      = opp_color(my_color);

    Board board;
    StateTable state_table;  // 局面の出現回数テーブル
    // 黒は先攻なので相手手の入力を最初だけスキップ
    // 黒番は先攻なので最初の手を指す前に相手の手を受け取る必要がない
    // 2手目以降は通常通り相手の手を受け取ってから指す
    bool skip_input = (my_color == BLACK);

    // ── メインループ ──────────────────────────────────
    while (true) {
        if (!skip_input) {
            if (!std::getline(std::cin, line)) break;
            line = trim(line);
            // rfind("GAME_OVER", 0) == 0: 文字列が "GAME_OVER" で始まるかの判定
            if (line.rfind("GAME_OVER", 0) == 0) break;

            Move opp_mv;
            if (!parse_move(line, opp_mv)) break;
            board.apply_move(opp_mv.r1, opp_mv.c1, opp_mv.r2, opp_mv.c2, opp);
            // 相手が指した後の局面を記録
            state_table[board.hash()]++;
        }

        // 自分の手を選んで指す
        Move mv = choose_best_move(board, my_color, TIME_LIMIT, state_table);
        board.apply_move(mv.r1, mv.c1, mv.r2, mv.c2, my_color);
        // 自分が指した後の局面を記録
        state_table[board.hash()]++;
        // "\n" だけではバッファに溜まって arena に届かない場合がある
        // flush で強制送信することで arena がすぐ手を受け取れる
        std::cout << mv.r1 << mv.c1 << mv.r2 << mv.c2 << "\n" << std::flush;

        skip_input = false;
    }

    return 0;
}