#!/bin/sh

# QTCLING_QT_ROOT overrides this default for an individual invocation.
if [ -z "${QTCLING_DEFAULT_QT_ROOT:-}" ]; then
    case "$(uname -s):$(uname -m)" in
        Darwin:*)                 qtcling_kit=macos ;;
        Linux:x86_64)             qtcling_kit=gcc_64 ;;
        Linux:aarch64|Linux:arm64) qtcling_kit=gcc_arm64 ;;
        *)                        qtcling_kit= ;;
    esac
    QTCLING_DEFAULT_QT_ROOT=${qtcling_kit:+/usr/local/qt/Qt/6.12.0/$qtcling_kit}
    unset qtcling_kit
fi
