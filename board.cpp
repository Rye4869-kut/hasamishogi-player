#include "Board.h"
#include <queue>
#include <vector>

// ── 初期化 ───────────────────────────────────────────
Board::Board() {
    memset(cells, EMPTY, sizeof(cells));
    for (int c = 0; c < BS; c++) {
        cells[0][c]    = BLACK;   // 黒は 0 行目
        cells[BS-1][c] = WHITE;   // 白は 8 行目
    }
    captures[0] = captures[1] = 0;
    pending_leader = EMPTY;
    turn = BLACK;
    last_move = {0, 0, 0, 0};
}

// ── is_clear_path ─────────────────────────────────────
// (r1,c1)→(r2,c2) の間に駒がないか確認（直線のみ）
bool Board::is_clear_path(int r1, int c1, int r2, int c2) const {
    if (r1 == r2) {
        int step = (c2 > c1) ? 1 : -1;
        for (int c = c1 + step; c != c2; c += step)
            if (cells[r1][c] != EMPTY) return false;
        return true;
    }
    if (c1 == c2) {
        int step = (r2 > r1) ? 1 : -1;
        for (int r = r1 + step; r != r2; r += step)
            if (cells[r][c1] != EMPTY) return false;
        return true;
    }
    return false; // 斜め → 不正
}

// ── capture_from ──────────────────────────────────────
// (r0,c0) から方向 (dr,dc) へ相手駒を探し、自駒で挟めたら取り除く
// Python: capture_from の忠実移植
int Board::capture_from(int r0, int c0, int dr, int dc, char me, char opp) {
    int r = r0 + dr, c = c0 + dc;

    // 相手駒の連続をリストアップ
    int buf_r[BS], buf_c[BS];
    int cnt = 0;
    while (in_bounds(r, c) && cells[r][c] == opp) {
        buf_r[cnt] = r;
        buf_c[cnt] = c;
        cnt++;
        r += dr; c += dc;
    }
    // 連続の末尾が自駒なら取れる
    if (cnt > 0 && in_bounds(r, c) && cells[r][c] == me) {
        for (int i = 0; i < cnt; i++)
            cells[buf_r[i]][buf_c[i]] = EMPTY;
        return cnt;
    }
    return 0;
}

// ── remove_dead_groups ────────────────────────────────
// 囲碁的ルール: 自由度(隣接空マス)がない連結グループを除去
// Python: remove_dead_groups の忠実移植
int Board::remove_dead_groups(char color) {
    bool visited[BS][BS] = {};
    int total = 0;

    for (int r = 0; r < BS; r++) {
        for (int c = 0; c < BS; c++) {
            if (visited[r][c] || cells[r][c] != color) continue;

            // BFS でグループと自由度を調べる
            std::vector<std::pair<int,int>> group;
            std::queue<std::pair<int,int>>  q;
            bool has_liberty = false;

            visited[r][c] = true;
            q.push({r, c});
            group.push_back({r, c});

            while (!q.empty()) {
                auto [cr, cc] = q.front(); q.pop();
                for (int d = 0; d < 4; d++) {
                    int nr = cr + DR[d], nc = cc + DC[d];
                    if (!in_bounds(nr, nc)) continue;
                    if (cells[nr][nc] == EMPTY)          { has_liberty = true; }
                    else if (cells[nr][nc] == color && !visited[nr][nc]) {
                        visited[nr][nc] = true;
                        q.push({nr, nc});
                        group.push_back({nr, nc});
                    }
                }
            }

            if (!has_liberty) {
                for (auto [gr, gc] : group)
                    cells[gr][gc] = EMPTY;
                total += (int)group.size();
            }
        }
    }
    return total;
}

// ── is_legal_move ─────────────────────────────────────
// Python: is_legal_move の忠実移植
// const だが suicide チェックで内部コピー上で変更処理を行う
bool Board::is_legal_move(int r1, int c1, int r2, int c2, char me) const {
    char opp = opp_color(me);

    // 1. 盤面内か
    if (!in_bounds(r1, c1) || !in_bounds(r2, c2)) return false;
    // 2. 自駒を空マスへ
    if (cells[r1][c1] != me || cells[r2][c2] != EMPTY) return false;
    // 3. 縦または横の直線移動（実際に移動している）
    if (!((r1 == r2 && c1 != c2) || (c1 == c2 && r1 != r2))) return false;
    // 4. 経路が空いている
    if (!is_clear_path(r1, c1, r2, c2)) return false;

    // 5. 自殺手チェック:
    //    移動先の対称位置に相手駒が両側にあるなら、
    //    何かを取れなければ非合法（自殺）
    for (int d = 0; d < 4; d++) {
        int rp = r2 + DR[d], cp = c2 + DC[d];  // 正方向
        int rn = r2 - DR[d], cn = c2 - DC[d];  // 負方向
        if (!in_bounds(rp, cp) || !in_bounds(rn, cn)) continue;
        if (cells[rp][cp] != opp || cells[rn][cn] != opp) continue;

        // Python と同様: 盤面コピー上でキャプチャをシミュレート
        // Board はPODなので = で高速コピー
        Board sim = *this;
        int total = 0;
        for (int d2 = 0; d2 < 4; d2++)
            total += sim.capture_from(r2, c2, DR[d2], DC[d2], me, opp);
        total += sim.remove_dead_groups(opp);
        if (total == 0) return false;
    }
    return true;
}

// ── apply_move ────────────────────────────────────────
// Python: apply_move の忠実移植
void Board::apply_move(int r1, int c1, int r2, int c2, char me) {
    char opp = opp_color(me);

    // 駒を移動
    cells[r1][c1] = EMPTY;
    cells[r2][c2] = me;

    // 取り駒処理
    int total = 0;
    for (int d = 0; d < 4; d++)
        total += capture_from(r2, c2, DR[d], DC[d], me, opp);
    total += remove_dead_groups(opp);
    captures[color_idx(me)] += total;

    // pending_leader 更新
    // (3枚差 → 猶予1手ルール)
    int cm = captures[color_idx(me)];
    int co = captures[color_idx(opp)];
    int lead = cm - co;
    if (lead >= 3 && pending_leader == EMPTY)
        pending_leader = me;
    else if (lead >= -2 && pending_leader == opp)
        pending_leader = EMPTY;

    last_move = {r1, c1, r2, c2};
    turn = opp;
}

// ── generate_legal_moves ──────────────────────────────
// Python: generate_legal_moves の忠実移植
std::vector<Move> Board::generate_legal_moves(char me) const {
    std::vector<Move> moves;
    moves.reserve(128);

    for (int r = 0; r < BS; r++) {
        for (int c = 0; c < BS; c++) {
            if (cells[r][c] != me) continue;
            for (int d = 0; d < 4; d++) {
                int nr = r + DR[d], nc = c + DC[d];
                while (in_bounds(nr, nc) && cells[nr][nc] == EMPTY) {
                    if (is_legal_move(r, c, nr, nc, me))
                        moves.push_back({r, c, nr, nc});
                    nr += DR[d]; nc += DC[d];
                }
            }
        }
    }
    return moves;
}

// ── is_game_over ──────────────────────────────────────
// Python: is_game_over の忠実移植
// 戻り値: 勝者 (BLACK / WHITE)、未決着なら EMPTY
char Board::is_game_over() const {
    char opp = turn;                     // 次に指すプレイヤー
    char me  = opp_color(opp);           // 直前に指したプレイヤー
    int  cm  = captures[color_idx(me)];

    if (cm >= 5)              return me;       // 5枚差で即勝利
    if (pending_leader == opp) return pending_leader; // 猶予を活かせなかった側の勝利
    return EMPTY;
}
