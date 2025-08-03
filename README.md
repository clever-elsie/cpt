[![Build Status](https://img.shields.io/github/actions/workflow/status/clever-elsie/cpt/cmake.yml?branch=main)](https://github.com/clever-elsie/cpt/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/clever-elsie/cpt)](https://github.com/clever-elsie/cpt/issues)
[![Pull Requests](https://img.shields.io/github/issues-pr/clever-elsie/cpt)](https://github.com/clever-elsie/cpt/pulls)

# cpt

## 概要
簡単な数式やLaTeX的な $\sum,\prod$ を計算するプログラムです。
関数定義を実装していますが，条件分岐で停止する再帰関数は無限ループします．  

## 特徴
- 多倍長整数・多倍長浮動小数点による四則演算
- 論理演算、べき乗、log、三角関数、$\sum, \prod$ などの計算
- 変数宣言も可能

## 動作環境・依存
- C++20 以降
- Boost.Multiprecision ライブラリ

## インストール方法
### CMakeを使う場合
```sh
mkdir build
cd build
cmake ..
make
make install  # ~/.local/bin などにインストールされます
```

### CMakeを使わずmakeでビルドする場合
```sh
make
```
ルート直下に実行ファイルが作成されます。

## 使い方
```sh
cpt 数式
```
- 数式は複数の式をセミコロンで区切って入力できます（表示されるのは最後の式のみ）。
- ファイルをパイプして入力することも可能です。
- 引数に`-f`とファイル名を付けると，そのファイルを入力とする．ただし，与えるファイル名が複数ある時は未定義動作．

## 実行例

```
<< cpt 1+2*3/4
>> 2.5

<< cpt "let x=10**5; x^2"
>> 1e+10

<< cpt let x=10, let y=2x, x^y
>> 1e+20

<< cpt 5 10 ^ 2 3
>> 5e+06

<< cpt "let x=10, 1<x<20"
>> true

<< cpt "1<2 & 2<3"
>> true

<< cpt "1>2 | 3>9"
>> false

<< cpt "let n=15;\sum_{i=1}^{n} i"
>> 120

<< cpt "\log_2 7"
>> 2.80735

<< cpt "def pow2(x){\
  x*x\
}\
pow2(pow2(4))"
>> 256

```
四則演算 `+ - * / //` とべき乗 `^ **` をサポート．
`/`は浮動小数点に自動昇格，`//`は整数割り切り，べき乗は浮動小数点に自動昇格．  
優先順位は `*`省略掛け算>べき乗>`*/%`>`+-`  

それよりも更に低い優先順位で`< <= > >= = == !=`が存在し，これらの優先順位は等価．  
比較演算子`=`と`==`は同じ．  
比較演算子は連続して二項評価されるが，falseが一つでもあれば短絡でfalseを返す．

それよりも低い優先順位で`& && | ||`が存在し，これらは論理積，論理和．

三項演算子で条件分岐も可能．使い方はC言語と同じ．

定義済み関数は三角関数，累積和，累積積，絶対値，`print`など．詳しくは <a href="./定義済み関数.md">関数一覧</a>で．


## ライセンス
MIT License

## バグ報告・コントリビュート
- バグ報告は issue へお願いします。
- 簡単な修正は pull request を送っていただければ merge します。
- 新機能の提案やテスト追加も issue で受け付けます。 