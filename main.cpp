#include <iostream>
#include <string>
#include "board.h"
#include "search.h"

static const double TIME_LIMIT = 1.0;  // 1手あたりの思考時間 (秒)
static const char*  PLAYER_NAME = "Violence Takeda";  // ← 自分の名前に変える

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
    // 黒は先攻なので相手手の入力を最初だけスキップ
    bool skip_input = (my_color == BLACK);

    // ── メインループ ──────────────────────────────────
    while (true) {
        if (!skip_input) {
            // 相手の指し手を受け取る (または GAME_OVER)
            if (!std::getline(std::cin, line)) break;
            line = trim(line);
            if (line.rfind("GAME_OVER", 0) == 0) break;

            Move opp_mv;
            if (!parse_move(line, opp_mv)) break;  // 不正 → 終了
            board.apply_move(opp_mv.r1, opp_mv.c1, opp_mv.r2, opp_mv.c2, opp);
        }

        // 自分の手を選んで指す
        Move mv = choose_best_move(board, my_color, TIME_LIMIT);
        board.apply_move(mv.r1, mv.c1, mv.r2, mv.c2, my_color);
        std::cout << mv.r1 << mv.c1 << mv.r2 << mv.c2 << "\n" << std::flush;

        skip_input = false;
    }

    return 0;
}