# 組込み関数仕様

## print
```
\print(expr,...)
```
- 式の値を表示する
- 成功時，表示した値の数を返す
- 失敗時は `exit(EXIT_FAILURE)` で即時終了

## 絶対値
```
\abs(expr)
```
- 絶対値を返す

## 切り下げ・切り上げ・四捨五入
```
\ceil(expr)
\floor(expr)
\round(expr)
\trunc(expr)
```
上から，切り上げ，切り下げ，四捨五入，0に近い方への丸め．

## 対数
```
\log(expr)
\log_{底}(expr)
\log10(expr)
```
- 下付きが与えられたとき，それを底とする．なければ自然対数（底 e）
- `\log10` は底 10

## 三角関数・双曲線関数
```
\sin(expr)  \cos(expr)  \tan(expr)
\asin(expr) \acos(expr) \atan(expr)
\sinh(expr) \cosh(expr) \tanh(expr)
```
- それぞれの関数値を返す

## 累積和・累積積
```
\sum_{i=0}^{n} expr
\prod_{x=99}^1000(expr)
```
- `\sum` は累積和，`\prod` は累積積
- 上付き・下付きで範囲や変数を指定
- ループ内では引数に `()` は不要

## ユーザー定義関数
```
def functionName(args,...){
  expr;
  ...
}
```
- 任意の式を関数として定義可能
- スコープ・引数管理あり

## 優先順位付き関数呼び出し
- `fn^{n+1}(arg)` や `fn^x(arg)` のように，べき乗を前方に優先させることができる

---

### 実装済み組み込み関数一覧
- `\print`
- `\abs`
- `\log`, `\log10`
- `\sin`, `\cos`, `\tan`
- `\asin`, `\acos`, `\atan`
- `\sinh`, `\cosh`, `\tanh`
- `\sum`, `\prod`
- `\ceil`, `\floor`, `\round`, `\trunc`



