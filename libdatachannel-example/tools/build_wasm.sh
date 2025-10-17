#!/bin/bash
set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Usage: $0 <emscripten-toolchain-file> [build-dir]"
    exit 1
fi

TOOLCHAIN_FILE=$1
BUILD_DIR=${2:-build-wasm}

cmake -S "$(dirname "$0")/.." -B "$BUILD_DIR" -DENABLE_WASM=ON -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

echo "WASM application built in $BUILD_DIR/wasm_app"

