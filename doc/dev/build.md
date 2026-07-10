# ビルドと開発環境

## ツールチェーン
- C++23（`CMAKE_CXX_STANDARD 23`）
- CMake 3.10+
- Boost（find_package(Boost REQUIRED)）
- jpp（find_package(jpp REQUIRED)）
  - リポジトリ: https://github.com/clever-elsie/jpp

## 依存ライブラリのセットアップ

### jpp ライブラリのインストール
cpt は JSON シリアライズのために `jpp` ライブラリを使用しています。ビルドする前に、以下の手順でシステムまたはユーザーローカル環境に `jpp` をインストールしてください。

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
- `CMAKE_INSTALL_PREFIX`（既定 `/usr/local`）

## Make ターゲット（ラッパー）
- `make` / `make build` / `make release-build`: Release ビルド
- `make debug` / `make debug-build`: Debug ビルド（`gdb ./build/cpt` 起動）
- `make test`: `scripts/test-runner.sh` 実行
- `make install PREFIX=/path`: インストールし，`build/.installed_prefix` に記録
- `make uninstall`: 記録に基づき削除
- `make update`: スクリプト経由で再インストール

## CI
- GitHub Actions ワークフロー（`ci.yml`）でビルド・テスト
