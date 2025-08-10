# ビルドと開発環境

## ツールチェーン
- C++23（`CMAKE_CXX_STANDARD 23`）
- CMake 3.10+
- Boost（find_package(Boost REQUIRED)）

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
