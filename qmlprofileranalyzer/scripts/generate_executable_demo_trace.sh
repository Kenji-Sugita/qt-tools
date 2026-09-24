#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_FILE="${1:-${ROOT_DIR}/samples/generated/localizedclockdemo.qtd}"

QMLPROFILER_BIN="${QMLPROFILER_BIN:-/usr/local/qt/Qt/6.11.0/macos/bin/qmlprofiler}"
DEMO_BIN="${DEMO_BIN:-${ROOT_DIR}/build/localizedclockdemo}"
PORT="${PORT:-37683}"
SERVICES="${SERVICES:-CanvasFrameRate,EngineControl,DebugMessages}"

mkdir -p "$(dirname "${OUTPUT_FILE}")"

if [[ ! -x "${QMLPROFILER_BIN}" ]]; then
    echo "qmlprofiler not found: ${QMLPROFILER_BIN}" >&2
    exit 1
fi

if [[ ! -x "${DEMO_BIN}" ]]; then
    echo "demo executable not found: ${DEMO_BIN}" >&2
    exit 1
fi

echo "Generating trace: ${OUTPUT_FILE}"

"${DEMO_BIN}" \
    -platform offscreen \
    "-qmljsdebugger=port:${PORT},block,services:${SERVICES}" &
DEMO_PID=$!

cleanup() {
    if kill -0 "${DEMO_PID}" 2>/dev/null; then
        kill "${DEMO_PID}" 2>/dev/null || true
        wait "${DEMO_PID}" 2>/dev/null || true
    fi
}

trap cleanup EXIT

"${QMLPROFILER_BIN}" \
    --attach localhost \
    --port "${PORT}" \
    --output "${OUTPUT_FILE}"

wait "${DEMO_PID}"
trap - EXIT

echo "Trace written to: ${OUTPUT_FILE}"
