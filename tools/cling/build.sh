#!/bin/sh
set -eu

SCRIPT_DIRECTORY=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
# shellcheck source=bin/qtcling-defaults.sh
. "$SCRIPT_DIRECTORY/bin/qtcling-defaults.sh"
export QTCLING_DEFAULT_QT_ROOT

INSTALL_PREFIX=/usr/local/qtcling
CMAKE_PLATFORM_FLAGS=
CLING_REVISION=af630d98
LLVM_REVISION=7c49650f1446

if [ "$(uname -s)" = "Darwin" ]; then
  MACOSX_SDK="$(xcrun --sdk macosx --show-sdk-path)"
  CMAKE_PLATFORM_FLAGS="
    -DPKG_CONFIG_EXECUTABLE=/usr/bin/false
    -DLibEdit_INCLUDE_DIRS=${MACOSX_SDK}/usr/include
    -DLibEdit_LIBRARIES=${MACOSX_SDK}/usr/lib/libedit.tbd"
fi

checkout_required_revision() {
  source_dir=$1
  revision=$2

  current_revision=$(git -C "$source_dir" rev-parse HEAD)
  required_revision=$(git -C "$source_dir" rev-parse "$revision^{commit}") || {
    echo "ERROR: $source_dir does not contain required revision $revision." >&2
    echo "ERROR: Remove the clean checkout and rerun this script to clone it again." >&2
    exit 1
  }

  if [ "$current_revision" = "$required_revision" ]; then
    return
  fi

  if ! git -C "$source_dir" diff --quiet ||
     ! git -C "$source_dir" diff --cached --quiet; then
    echo "ERROR: $source_dir is at $current_revision, but qtcling requires $required_revision." >&2
    echo "ERROR: Commit, stash, or discard its changes before rerunning this script." >&2
    exit 1
  fi

  echo "Checking out $source_dir at $required_revision."
  git -C "$source_dir" checkout --detach "$required_revision"
}

if [ ! -d cling ]; then
  git clone https://github.com/root-project/cling.git cling
fi
checkout_required_revision cling "$CLING_REVISION"

if [ ! -d llvm-project ]; then
  git clone https://github.com/root-project/llvm-project.git llvm-project
fi
checkout_required_revision llvm-project "$LLVM_REVISION"

apply_patch_if_needed() {
  patch_file=$1
  marker_file=$2
  marker_text=$3

  if git apply --check "$patch_file" 2>/dev/null; then
    git apply "$patch_file"
  elif [ -f "$marker_file" ] && grep -q "$marker_text" "$marker_file"; then
    echo "$patch_file is already applied."
  else
    echo "ERROR: $patch_file cannot be applied and is not already applied." >&2
    echo "ERROR: Check that cling/ and llvm-project/ are clean and at the expected revisions." >&2
    exit 1
  fi
}

apply_required_patch() {
  patch_file=$1
  marker_file=$2
  marker_text=$3

  if [ ! -f "$patch_file" ]; then
    echo "ERROR: missing required patch: $patch_file" >&2
    exit 1
  fi
  apply_patch_if_needed "$patch_file" "$marker_file" "$marker_text"
}

apply_required_patch \
  patch/qtcling-interactive/0001-cling-add-periodic-callback-api.patch \
  cling/include/cling/Interpreter/Interpreter.h \
  cling_set_periodic_callback
apply_required_patch \
  patch/qtcling-interactive/0002-llvm-lineeditor-periodic-callback.patch \
  llvm-project/llvm/include/llvm/LineEditor/LineEditor.h \
  setPeriodicCallback
apply_required_patch \
  patch/qtcling-interactive/0003-cling-load-qtcling-startup-file.patch \
  cling/lib/UserInterface/UserInterface.cpp \
  QTCLING_STARTUP_FILE
apply_required_patch \
  patch/qtcling-interactive/0006-warn-when-libedit-is-disabled.patch \
  cling/CMakeLists.txt \
  "qtcling interactive needs LLVM libedit"

mkdir -p build
cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
  -DLLVM_ENABLE_PROJECTS=clang \
  -DLLVM_EXTERNAL_PROJECTS=cling \
  -DLLVM_EXTERNAL_CLING_SOURCE_DIR=../cling \
  -DLLVM_BUILD_TOOLS=OFF \
  -DLLVM_TARGETS_TO_BUILD=host \
  ${CMAKE_PLATFORM_FLAGS} \
  ../llvm-project/llvm

if grep -q '^LLVM_ENABLE_LIBEDIT:BOOL=OFF' CMakeCache.txt; then
  echo "WARNING: LLVM_ENABLE_LIBEDIT is OFF." >&2
  echo "WARNING: qtcling interactive will not process Qt events while waiting for REPL input." >&2
  echo "WARNING: Install libedit development files, for example 'sudo apt install libedit-dev' on Ubuntu/WSL, then rebuild from a clean build directory." >&2
fi

cmake --build . -j 8
