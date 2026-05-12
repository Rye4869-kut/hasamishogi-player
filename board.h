#pragma once
#include <vector>
#include <cstring>

// ── 定数 ──────────────────────────────────────────────
static const int BS    = 9;
static const char EMPTY = '.';
static const char BLACK = 'B';
static const char WHITE = 'W';

// 4方向 (下,上,右,左)
static const int DR[4] = { 1, -1,  0,  0 };
static const int DC[4] = { 0,  0,  1, -1 };

// ── ユーティリティ ──────────────────────────────────
inline bool  in_bounds (int r, int c) { return r >= 0 && r < BS && c >= 0 && c < BS; }
inline char  opp_color (char c)       { return c == BLACK ? WHITE : BLACK; }
inline int   color_idx (char c)       { return c == BLACK ? 0 : 1; }

// ── 指し手 ────────────────────────────────────────────
struct Move {
    int r1, c1, r2, c2;
};

// ── 盤面状態 ──────────────────────────────────────────
// POD 構造体なので Board copy = board; が高速なコピーになる
struct Board {
    char cells[BS][BS];   // EMPTY / BLACK / WHITE
    int  captures[2];     // [0]=BLACK の取り駒数, [1]=WHITE の取り駒数
    char pending_leader;  // EMPTY, BLACK, WHITE
    char turn;            // 次に指すプレイヤー
    Move last_move;

    // 初期配置で初期化
    Board();

    // ── ゲームロジック (hasamiShogi.py の忠実移植) ──
    bool is_clear_path  (int r1, int c1, int r2, int c2) const;
    int  capture_from   (int r0, int c0, int dr, int dc, char me, char opp);
    int  remove_dead_groups(char color);
    bool is_legal_move  (int r1, int c1, int r2, int c2, char me) const;
    void apply_move     (int r1, int c1, int r2, int c2, char me);

    std::vector<Move> generate_legal_moves(char me) const;
    char is_game_over() const;  // 勝者を返す。決着なし → EMPTY
};
