[![Build Status](https://img.shields.io/github/actions/workflow/status/clever-elsie/cpt/ci.yml?branch=main)](https://github.com/clever-elsie/cpt/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/clever-elsie/cpt)](https://github.com/clever-elsie/cpt/issues)
[![Pull Requests](https://img.shields.io/github/issues-pr/clever-elsie/cpt)](https://github.com/clever-elsie/cpt/pulls)

# cpt

## 概要
cptは多倍長整数・多倍長浮動小数点による数式計算，LaTeX風の\sum, \prod，ユーザー定義関数，スコープ付き変数，論理・比較・三項演算などをサポートする数式処理系です．

## 特徴
- 多倍長整数・多倍長浮動小数点による計算
- 変数宣言・スコープ管理
- ユーザー定義関数（再帰可）
- 組み込み関数（print, abs, log, 三角関数, 累積和/積 など）
- LaTeX風の\sum, \prod, \log記法
- 論理演算・比較演算・三項演算子
- セミコロン区切りで複数式を記述可能
- 標準入力・ファイル入力・パイプ入力対応

## 動作環境・依存
- C++23 以降
- Boost.Multiprecision ライブラリ

## インストール方法
依存パッケージの導入例:
```sh
# Ubuntu / Debian（WSL を含む）
sudo apt update
sudo apt install -y g++-14 build-essential cmake libboost-all-dev
make install  # /usr/local/bin にインストールされます
```

### アップデート
```sh
sudo make update
```
`git`が`sudo`で使えるならこの方法でリモートの更新を自動で反映する．  
もしインストール場所が`sudo`無しで書き込みできるなら`make update`でいい．

### アンインストール
```sh
make uninstall
```

### ビルド
```sh
make           # Release ビルド
make build     # Release ビルド
make debug     # Debug ビルド（gdb 起動ターゲットあり）
```

## ドキュメント
このプロジェクトの公式ドキュメントは `doc/` 配下にあります．主な入口は以下です．

### ユーザー向け
- [組み込み関数仕様](doc/spec/functions.md)

### 開発者向け
- [ビルドと開発環境](doc/dev/build.md)
- [アーキテクチャ概要](doc/dev/architecture.md)
- [テストガイド](doc/dev/testing.md)
- [コントリビュートガイド](doc/dev/contributing.md)

---
リンク切れや内容の不備があれば issue/PR にてご報告ください．


## 使い方
```sh
cpt 数式
```
- 数式はセミコロン区切りで複数記述可能（表示は最後の式のみ）
- ファイルやパイプからの入力も可能
- `-f ファイル名`でファイル入力（複数ファイルは未定義動作）
- コマンドラインで数式を与えるとき，`'`で文字列リテラルとして与えるべき．(`"`で囲むとシェルが変数などを自動展開するため非推奨)

最後に実行した式の値か，`\print()`に与えた引数の値を表示する．
`\print()`は可変長引数関数で，戻り値は表示した引数の数なので最後に`\print()`を実行すると表示した引数の数も表示される．

インストールせずに実行する場合（ビルド済み）:
```sh
./build/cpt '1+2*3/4'
```

ファイル入力:
```sh
./build/cpt -f sample.cpt
```

パイプ入力:
```sh
echo "let x=10, x^2" | ./build/cpt
```

標準入力:
```
./build/cpt
def f(n){
  0<=n<=1 ? 1 : n f(n-1) ;
}
f(1000)
```

## 言語仕様
### データ型
- 多倍長整数, 多倍長浮動小数点, 真偽値

### 変数宣言・代入
- `let x=expr;` または `let x:=expr;` で宣言
- 代入は `:=`（`=`は宣言時のみ）
- スコープは関数・ブロック単位で管理

### 演算子・優先順位
- 四則演算: `+ - * / // %`（`/`は浮動小数点, `//`は割り切り除算）
- べき乗: `^` `**`（右結合）
- 階乗: `!`
- 比較: `< <= > >= = == !=`（`=`と`==`は同じ，連続比較・短絡評価）
- 論理: `& && | ||`（短絡評価）
- 三項演算: `条件 ? 真 : 偽`（C言語と同じ）
- 代入: `:=` (右結合)
- 優先順位: べき乗=階乗 > * / // % > + - > 比較 > 論理 > 三項 > 代入

### 関数定義・呼び出し
- `def 関数名(引数,...) { 式; ... }`
- 関数内でローカル変数・引数がスコープ管理される
- 再帰関数可（停止条件がなければ無限ループ）
- 組み込み関数は\print, \abs, \log, \sum, \prod, 三角関数など
- 関数は`()`の前に`^{}`でべき乗を前置にできる

### LaTeX風記法
- `\sum_{i=0}^{n} expr` 累積和
- `\prod_{i=1}^{n} expr` 累積積
- `\log_{底}(expr)` 対数

### 組み込み関数
- 詳細は`doc/spec/functions.md`参照
- 例: `\print(expr,...)`, `\abs(expr)`, `\log(expr)`, `\sin(expr)` など

### スコープ・意味解析
- 変数・関数の重複チェック，予約語衝突検出
- 関数定義は関数内で再定義不可
- 変数宣言時のスコープ管理
- 構文・意味エラー時は例外スロー

### エラー処理
- 例外スローによるエラー通知
- printやエラー時は即時終了

## ライセンス
MIT License

## 開発・テスト

### ローカルでのテスト実行
```sh
make test
```

### GitHub Actions
このプロジェクトはGitHub Actionsで自動テストが設定されています：

- **CI**: 基本的なビルドとテスト（`ci.yml`）

プッシュやプルリクエスト時に自動的にテストが実行されます．

## バグ報告・コントリビュート
- バグ報告は issue へお願いします．
- 簡単な修正は pull request を送っていただければ merge します．
- 新機能の提案やテスト追加も issue で受け付けます． 