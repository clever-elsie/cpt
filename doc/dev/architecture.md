# アーキテクチャ概要（開発者向け）

ソース構成（`src/`）の主なサブディレクトリ：
- `src/tokenizer` トークナイザ
- `src/parser` 構文解析
- `src/ast` 抽象構文木（`reserved/` には予約語/組み込み）
- `src/type` 型・評価関連
- `src/input` 入力（標準入力/ファイル/パイプ）

ビルドは CMake で全 `src/*.cpp` を収集し単一実行ファイル `cpt` を生成します．

機能拡張やバグの修正などの提案はすべてissuesで行います．
