#!/usr/bin/env bash
set -euo pipefail

source_root="$1"
repl_file="$source_root/src/repl.cpp"
max_repl_lines=200

line_count=$(wc -l < "$repl_file")
if (( line_count > max_repl_lines )); then
    echo "src/repl.cpp has $line_count lines; keep REPL behavior in class- or feature-named files." >&2
    exit 1
fi

required_files=(
    "$source_root/src/Application.cpp"
    "$source_root/src/InterpreterProcess.cpp"
    "$source_root/src/ReplSessionImpl.cpp"
    "$source_root/src/ReplSessionQtWidgets.cpp"
)

for file_path in "${required_files[@]}"; do
    if [[ ! -f "$file_path" ]]; then
        echo "Expected source split file is missing: $file_path" >&2
        exit 1
    fi
done
