#!/bin/sh
set -eu

SCRIPT_DIRECTORY=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT_DIRECTORY_WINDOWS=$(cygpath -w "$SCRIPT_DIRECTORY")
cd "$SCRIPT_DIRECTORY"

INSTALL_PREFIX="${SCRIPT_DIRECTORY_WINDOWS}\\qtcling-win-arm64"
BUILD_DIRECTORY=build-win-arm64
PYTHON_EXECUTABLE="${LOCALAPPDATA}\\Programs\\Python\\Python313-arm64\\python.exe"
CLING_REVISION=af630d98
LLVM_REVISION=7c49650f1446

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

if [ ! -f "$(cygpath -u "$PYTHON_EXECUTABLE")" ]; then
  echo "ERROR: ARM64 Python not found: $PYTHON_EXECUTABLE" >&2
  exit 1
fi

powershell.exe -NoProfile -ExecutionPolicy Bypass \
  -File "${SCRIPT_DIRECTORY_WINDOWS}\\build-libedit-win.ps1"

LIBEDIT_PREFIX_WINDOWS=$(cygpath -w "${SCRIPT_DIRECTORY}/libedit-win-arm64")

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
apply_required_patch \
  patch/qtcling-interactive/0007-enable-libedit-on-windows.patch \
  llvm-project/llvm/cmake/modules/FindLibEdit.cmake \
  _libedit_library_dir
apply_required_patch \
  patch/qtcling-interactive/0008-cling-windows-native.patch \
  cling/lib/MetaProcessor/MetaProcessor.cpp \
  '#ifndef _WIN32'
apply_required_patch \
  patch/qtcling-interactive/0009-windows-libedit-utf8-locale.patch \
  llvm-project/llvm/lib/LineEditor/LineEditor.cpp \
  'setlocale(LC_CTYPE, ".UTF-8")'

cmake -S llvm-project/llvm -B "$BUILD_DIRECTORY" \
  -G "Visual Studio 17 2022" \
  -A ARM64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
  -DLLVM_ENABLE_PROJECTS=clang \
  -DLLVM_EXTERNAL_PROJECTS=cling \
  -DLLVM_EXTERNAL_CLING_SOURCE_DIR="${SCRIPT_DIRECTORY_WINDOWS}\\cling" \
  -DCLING_BUILD_JUPYTER=OFF \
  -DLLVM_BUILD_TOOLS=OFF \
  -DLLVM_TARGETS_TO_BUILD=host \
  -DLLVM_ENABLE_LIBEDIT=FORCE_ON \
  -DPython3_EXECUTABLE="$PYTHON_EXECUTABLE" \
  -DCMAKE_PREFIX_PATH="${LIBEDIT_PREFIX_WINDOWS}" \
  -DCMAKE_C_FLAGS="-utf-8" \
  -DCMAKE_CXX_FLAGS="-utf-8"

cmake --build "$BUILD_DIRECTORY" --config Release -j 2
cmake --build "$BUILD_DIRECTORY" --config Release --target install -j 2
