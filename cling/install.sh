#!/bin/sh
set -eu

SCRIPT_DIRECTORY=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIRECTORY=${QTCLING_BUILD_DIR:-"$SCRIPT_DIRECTORY/build"}
CACHE_FILE="$BUILD_DIRECTORY/CMakeCache.txt"

if [ ! -f "$CACHE_FILE" ]; then
  echo "ERROR: CMake cache not found: $CACHE_FILE" >&2
  echo "ERROR: Build qtcling first with ./build.sh." >&2
  exit 1
fi

INSTALL_PREFIX=$(sed -n 's/^CMAKE_INSTALL_PREFIX:PATH=//p' "$CACHE_FILE")
if [ -z "$INSTALL_PREFIX" ]; then
  echo "ERROR: CMAKE_INSTALL_PREFIX is not set in $CACHE_FILE." >&2
  exit 1
fi

cmake --install "$BUILD_DIRECTORY"

install -d -m 755 "$INSTALL_PREFIX/bin" "$INSTALL_PREFIX/src"
install -m 755 \
  "$SCRIPT_DIRECTORY/bin/qtcling" \
  "$SCRIPT_DIRECTORY/bin/iqtcling.linux" \
  "$SCRIPT_DIRECTORY/bin/iqtcling.macos" \
  "$SCRIPT_DIRECTORY/bin/cqtcling.linux" \
  "$SCRIPT_DIRECTORY/bin/cqtcling.macos" \
  "$SCRIPT_DIRECTORY/bin/qtcling-defaults.sh" \
  "$INSTALL_PREFIX/bin/"
install -m 644 "$SCRIPT_DIRECTORY/src/qtgui.cpp" "$INSTALL_PREFIX/src/"
install -m 644 "$SCRIPT_DIRECTORY/QTCLING_VERSION" "$INSTALL_PREFIX/"

echo "qtcling installed to $INSTALL_PREFIX"
