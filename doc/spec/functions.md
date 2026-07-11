# 組込み関数・定数仕様

## 組込み定数
- `\pi`： 円周率 $\pi$
- `\e`： 自然対数の底 $e$
- `true` / `false`： 真偽値定数
- `\i`： 複素数の虚数単位 $i$
- `\inf`： 無限大 ($\infty$)
- `\nan`： 非数 (NaN)
- `\eps`： マシンイプシロン ($\epsilon$, 精度限界値)

## 組込み関数

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

## 標準入力
```
\read()
\input()
```
- 入力ストリームから1行ずつトークン/数値を読み込みます。

## 線形代数
```
\T(A)
\t(A)
\dot(A, B)
\vector(n)
\rowvector(n)
```
- `\T`: 行列の転置を返す。
- `\t`: 行列の複素共役転置を返す。
- `\dot`: 行列の要素ごとの積（アダマール積）を返す。
- `\vector(n)`: $n \times 1$ の列ベクトル（行列）を `0` で初期化して構築する。
- `\rowvector(n)`: $1 \times n$ の行ベクトル（行列）を `0` で初期化して構築する。

## イテレータアダプタ (遅延評価)
```
iterable | \take(k)
iterable | \drop(k)
iterable | \filter(pred)
iterable | \transform(pred)
iterable | \enumerate()
iterable | \reverse()
```
- `take(k)`: 先頭から `k` 個の要素を取得。
- `drop(k)`: 先頭から `k` 個の要素をスキップ。
- `filter(pred)`: 条件 `pred` に合致する要素を抽出。
- `transform(pred)`: 各要素に関数 `pred` を適用。
- `enumerate()`: 各要素とインデックスのペアを取得。
- `reverse()`: 逆順にする。

---

## 行列の次元
```
\rows(A)
\cols(A)
```
- `\rows`: 行列 `A` の行数を返す。
- `\cols`: 行列 `A` の列数を返す。
- 行列型以外に適用した場合は実行時エラーとなります。

## ディープコピー
```
\copy(mat)
\copy(mat[i])
\copy(mat, r_start..r_end, c_start..c_end)
\copy(vec, start..end)
```
- `\copy(mat)`: 行列全体のディープコピーを作成する。
- `\copy(mat[i])`: 指定した行ベクトルのディープコピーを作成する。
- `\copy(mat, r_start..r_end, c_start..c_end)`: 指定した2次元範囲の部分行列ディープコピーを作成する。
- `\copy(vec, start..end)`: ベクトルの指定範囲の部分ベクトルディープコピーを作成する。

## 参照の取得
```
\ref(expr)
```
- 式 `expr` (変数または行列要素) への参照型オブジェクト（`REF`）を返す。
- 参照オブジェクトに対する代入（`:=`）は、元の変数または行列要素を直接更新します。

## 行列イテレーション化
```
\as_mat(v)
```
- ベクトルなどのレイアウト情報を行列形式のまま保持し、`for` ループで要素ごとではなく行（行ベクトル）ごとにイテレーションを行うように強制する。

---

## 行列の添字アクセスと参照型スライス

### 添字アクセス
- `mat[i]`: 行列の `i` 行目にアクセスする（行ベクトルを返す）。ただし列ベクトルに対して適用した場合は `i` 番目の要素にアクセスする。
- `mat[i][j]`: 行列の `i` 行 `j` 列の要素にアクセスする（スカラへの参照を返す）。列ベクトルに対しては `j == 0` のみ許容される。
- `mat[i]` などの添字アクセスで返されるオブジェクトは**参照**であり、`mat[i] := new_row_vec` や `mat[i][j] := val` で元の行列自体を直接更新することが可能です。

### 範囲指定スライス
- `mat[r_start..r_end][c_start..c_end]`: 部分行列への参照スライスを返す。
- スライスに対して `mat[0..2][0..2] := 99` のように代入を行うと、元の行列の対応する範囲のすべての要素が直接更新されます。

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
- `\read`, `\input`
- `\T`, `\t`, `\dot`, `\vector`, `\rowvector`
- `\take`, `\drop`, `\filter`, `\transform`, `\enumerate`, `\reverse`
- `\rows`, `\cols`, `\copy`, `\ref`, `\as_mat`




