#!/usr/bin/env bash
set -euo pipefail

case_dir="$(pwd)"
repo_dir="$(cd "$(dirname "$0")/../../.." && pwd)"
icpp_bin="${ICPP_BIN:-$repo_dir/build/icpp}"

if [[ ! -x "$icpp_bin" ]]; then
    echo "icpp not found: $icpp_bin" >&2
    echo "ICPP_BIN を指定するか、先に icpp をビルドしてください。" >&2
    exit 1
fi

if [[ ! -f "$case_dir/input.icpp" ]]; then
    echo "$case_dir に input.icpp がありません。" >&2
    exit 1
fi

QTFRAMEWORK_BYPASS_LICENSE_CHECK="${QTFRAMEWORK_BYPASS_LICENSE_CHECK:-1}" \
    "$icpp_bin" < "$case_dir/input.icpp" > "$case_dir/output.txt" 2>&1
echo "作成しました: $case_dir/output.txt"
