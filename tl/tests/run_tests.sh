#!/usr/bin/env bash

set -euo pipefail

tl_bin="$1"

if [[ ! -x "$tl_bin" ]]; then
  echo "missing binary: $tl_bin" >&2
  exit 1
fi

if "$tl_bin" "one" "two" >/dev/null 2>&1; then
  echo "expected multiple positional arguments to fail" >&2
  exit 1
fi

if printf 'stdin\n' | "$tl_bin" --file /tmp/tl-test-input.txt >/dev/null 2>&1; then
  echo "expected file/stdin conflict to fail" >&2
  exit 1
fi

if "$tl_bin" >/tmp/tl-test-output.txt 2>/tmp/tl-test-error.txt; then
  echo "expected empty stdin to fail" >&2
  exit 1
fi

grep -q "empty input" /tmp/tl-test-error.txt
