#!/bin/bash

# テストランナースクリプト
# test/*.inファイルを./cptで実行し、test/*.outファイルと比較する

set -e  # エラーが発生したら終了

# 色付き出力のための関数
print_success() {
    echo -e "\033[32m✓ $1\033[0m"
}

print_error() {
    echo -e "\033[31m✗ $1\033[0m"
}

print_info() {
    echo -e "\033[34mℹ $1\033[0m"
}

# cpt実行ファイルが存在するかチェック（現在のディレクトリまたは親ディレクトリを確認）
if [ -f "./cpt" ]; then
    CPT_EXEC="./cpt"
elif [ -f "../cpt" ]; then
    CPT_EXEC="../cpt"
else
    print_error "cpt実行ファイルが見つかりません。先にビルドしてください。"
    exit 1
fi

# testディレクトリが存在するかチェック
if [ ! -d "test" ]; then
    print_error "testディレクトリが見つかりません。"
    exit 1
fi

# test/*.inファイルを検索
input_files=$(find test -name "*.in" -type f | sort)

if [ -z "$input_files" ]; then
    print_error "test/*.inファイルが見つかりません。"
    exit 1
fi

print_info "テストを開始します..."
echo

total_tests=0
passed_tests=0
failed_tests=0

# 各テストファイルを処理
for input_file in $input_files; do
    # 対応する出力ファイル名を生成
    output_file="${input_file%.in}.out"
    
    # 出力ファイルが存在するかチェック
    if [ ! -f "$output_file" ]; then
        print_error "対応する出力ファイルが見つかりません: $output_file"
        failed_tests=$((failed_tests + 1))
        continue
    fi
    
    total_tests=$((total_tests + 1))
    test_name=$(basename "$input_file" .in)
    
    echo "テスト: $test_name"
    
    # 一時ファイルを作成して出力を保存
    temp_output=$(mktemp)
    
    # cptでテストを実行
    if $CPT_EXEC < "$input_file" > "$temp_output" 2>&1; then
        # 出力を比較
        if diff -q "$temp_output" "$output_file" > /dev/null 2>&1; then
            print_success "テスト $test_name が成功しました"
            passed_tests=$((passed_tests + 1))
        else
            print_error "テスト $test_name が失敗しました"
            echo "期待される出力:"
            cat "$output_file"
            echo
            echo "実際の出力:"
            cat "$temp_output"
            echo
            failed_tests=$((failed_tests + 1))
        fi
    else
        print_error "テスト $test_name の実行中にエラーが発生しました"
        echo "実際の出力:"
        cat "$temp_output"
        echo
        failed_tests=$((failed_tests + 1))
    fi
    
    # 一時ファイルを削除
    rm -f "$temp_output"
    echo
done

# 結果を表示
echo "=== テスト結果 ==="
echo "総テスト数: $total_tests"
echo "成功: $passed_tests"
echo "失敗: $failed_tests"

if [ $failed_tests -eq 0 ]; then
    print_success "すべてのテストが成功しました！"
    exit 0
else
    print_error "$failed_tests 個のテストが失敗しました"
    exit 1
fi
