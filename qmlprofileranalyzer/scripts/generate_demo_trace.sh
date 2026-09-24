#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
QML_FILE="${ROOT_DIR}/samples/qml/AutoTrace.qml"
OUTPUT_FILE="${1:-${ROOT_DIR}/samples/generated/autotrace.qtd}"

QMLPROFILER_BIN="${QMLPROFILER_BIN:-/usr/local/qt/Qt/6.11.0/macos/bin/qmlprofiler}"
QML_BIN="${QML_BIN:-/usr/local/qt/Qt/6.11.0/macos/bin/qml}"
PORT="${PORT:-37680}"
SERVICES="${SERVICES:-CanvasFrameRate,EngineControl,DebugMessages}"

mkdir -p "$(dirname "${OUTPUT_FILE}")"

if [[ ! -x "${QMLPROFILER_BIN}" ]]; then
    echo "qmlprofiler not found: ${QMLPROFILER_BIN}" >&2
    exit 1
fi

if [[ ! -x "${QML_BIN}" ]]; then
    echo "qml not found: ${QML_BIN}" >&2
    exit 1
fi

echo "Generating trace: ${OUTPUT_FILE}"

"${QML_BIN}" \
    -platform offscreen \
    "-qmljsdebugger=port:${PORT},block,services:${SERVICES}" \
    "${QML_FILE}" &
QML_PID=$!

cleanup() {
    if kill -0 "${QML_PID}" 2>/dev/null; then
        kill "${QML_PID}" 2>/dev/null || true
        wait "${QML_PID}" 2>/dev/null || true
    fi
}

trap cleanup EXIT

"${QMLPROFILER_BIN}" \
    --attach localhost \
    --port "${PORT}" \
    --output "${OUTPUT_FILE}"

wait "${QML_PID}"
trap - EXIT

echo "Trace written to: ${OUTPUT_FILE}"
