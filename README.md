# はさみ将棋 AI

反復深化 Alpha-Beta 探索による C++ 実装のはさみ将棋 AI．

## セットアップ

### 1. リポジトリを clone する

```bash
git clone https://github.com/RLHS-Lab-KUT/hasamiShogi.git
cd hasamiShogi
```

### 2. 依存パッケージをインストールする

```bash
pip install pygame
```

### 3. AI をコンパイルする

```bash
cd players/Tanimoto
make
cd ../..
```

## ファイル構成

```
players/Tanimoto/
├── board.h / board.cpp   # 盤面・ゲームロジック
├── eval.h  / eval.cpp    # 評価関数
├── search.h/ search.cpp  # 探索エンジン（Alpha-Beta）
├── main.cpp              # arena との通信
└── Makefile              # ビルド設定
```

## アルゴリズム

### 探索
- **反復深化 Negamax Alpha-Beta**
  - 深さ1から始めて時間内に深さを増やしながら探索する
  - 完走した最も深い深さの結果を採用する
  - 思考時間：1手あたり1秒

- **手のオーダリング**
  - 取り駒が期待できる手を優先して探索することでカット効率を上げる

### 評価関数
- 取り駒差（最重要・×100）
- 中央支配（盤面中央3×3への駒の配置）
- pending_leader の状態（3枚差による猶予ルールの有利・不利）
- 繰り返しペナルティ（同一局面の3回出現を負けレベルで回避）

### 繰り返し検出
Zobrist ハッシュで局面を64ビット整数に変換し，出現回数を管理する．
3回目になる局面には INF のペナルティを与えて探索が避けるようにしている．

## 動作環境

- OS：Ubuntu（Linux）
- コンパイラ：g++ / C++17
- 依存ライブラリ：なし（標準ライブラリのみ）

## 実行方法

プロジェクトルートから実行する．

```bash
# ランダムプレイヤーと対戦
python arena.py "players/Tanimoto/player" "python randomPlayer.py"

# 自己対戦
python arena.py "players/Tanimoto/player" "players/Tanimoto/player"
```