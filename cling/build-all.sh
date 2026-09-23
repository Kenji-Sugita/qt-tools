#!/bin/sh
set -eu

CMAKE_PLATFORM_FLAGS=

if [ "$(uname -s)" = "Darwin" ]; then
  MACOSX_SDK="$(xcrun --sdk macosx --show-sdk-path)"
  CMAKE_PLATFORM_FLAGS="
    -DPKG_CONFIG_EXECUTABLE=/usr/bin/false
    -DLibEdit_INCLUDE_DIRS=${MACOSX_SDK}/usr/include
    -DLibEdit_LIBRARIES=${MACOSX_SDK}/usr/lib/libedit.tbd"
fi

mkdir build
cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS=clang \
  -DLLVM_EXTERNAL_PROJECTS=cling \
  -DLLVM_EXTERNAL_CLING_SOURCE_DIR=../cling \
  -DLLVM_BUILD_TOOLS=OFF \
  ${CMAKE_PLATFORM_FLAGS} \
  ../llvm-project/llvm

cmake --build . -j
