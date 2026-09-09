# ビルドと開発環境

## ツールチェーン
- C++23（`CMAKE_CXX_STANDARD 23`）
- CMake 3.30+
- Boost（システムにあれば使用し，なければCMakeが自動取得）
- jpp（システムにあれば使用し，なければCMakeが自動取得）
  - リポジトリ: https://github.com/clever-elsie/jpp

## 依存ライブラリのセットアップ

### jpp ライブラリのインストール
cpt は JSON シリアライズのために `jpp` ライブラリを使用しています。システムまたはユーザーローカル環境に
インストール済みの `jpp` があればそれを使用し、見つからない場合はCMakeが構成時に取得してビルドします。
オフラインで構成する場合や取得する版を自分で管理する場合は、以下の手順で事前にインストールしてください。

```bash
git clone https://github.com/clever-elsie/jpp.git
cd jpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# システム全体にインストールする場合（sudo 権限が必要）
sudo cmake --install build
# またはユーザーローカル環境にインストールする場合
cmake --install build --prefix ~/.local
```
※ `~/.local` にインストールした場合は、 cpt のビルド時に `-DCMAKE_PREFIX_PATH=~/.local` を指定するか、環境変数に追加してください。


## CMake オプション
- `-DCMAKE_BUILD_TYPE=Debug|Release`
- `CMAKE_INSTALL_PREFIX`（Unix の既定は `/usr/local`）
- `-DCPT_FETCH_BOOST=ON|OFF`（既定 `ON`。`OFF` では既存のBoostを必須にする）
- `-DCPT_BOOST_VERSION=x.y.z`（既定は最新安定版を問い合わせる）

Boostはまずシステムにインストールされたものを探します。見つからない場合は、既定で
Boostの最新安定版を問い合わせてソースアーカイブをダウンロードし、ヘッダーをビルドに利用します。
このため、Boostがシステムにない場合は構成時にネットワーク接続が必要です。特定版に固定する場合は
`-DCPT_BOOST_VERSION=1.92.0` のように指定できます。ネットワークから取得したくない場合は、
システムにBoostを用意してから次のように指定します。

最新追従モードでは構成時にGitHub APIへ問い合わせるため、オフライン構成には使えません。
ビルドを完全に再現したい場合は、`CPT_BOOST_VERSION` を指定して版を固定してください。

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCPT_FETCH_BOOST=OFF
```

## インストール先の指定

CMake では、インストール先のルートを `CMAKE_INSTALL_PREFIX` で指定します。
ユーザーディレクトリへインストールする場合は、次のように設定します。

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build
cmake --install build
```

既に構成済みのビルドに対して一時的にインストール先を変える場合は、
`cmake --install` の `--prefix` を使えます。

```bash
cmake --install build --prefix "$HOME/.local"
```

Makefile のラッパーを使う場合も同じ指定ができます。

```bash
make install PREFIX="$HOME/.local"
```

`/usr/local` にインストールする場合は、権限に応じて `sudo make install` を使います。
パッケージ作成用に別のディレクトリへ一時配置する場合は、`DESTDIR` を使います。

```bash
DESTDIR="$PWD/package-root" cmake --install build --prefix /usr
```

`DESTDIR` は実際のインストール先を変えず、`package-root/usr/bin/cpt` のような staging 用の配置を作ります。

## Make ターゲット（ラッパー）
- `make` / `make build` / `make release-build`: Release ビルド
- `make debug` / `make debug-build`: Debug ビルド（`gdb ./build/cpt` 起動）
- `make -jN build`: `-jN` をCMakeのビルドにも渡す
- `make test`: `scripts/test-runner.sh` 実行
- `make install PREFIX=/path`: インストールし，`build/.installed_prefix` に記録
- `make uninstall`: 記録に基づき削除
- `make update`: スクリプト経由で再インストール

`make` に `-j` を付けない場合は、CMakeにも並列オプションを渡しません。
`make -j` または `make -jN` を指定した場合だけ、CMakeにも同じ並列指定を渡します。

## CI
- GitHub Actions ワークフロー（`ci.yml`）でビルド・テスト
