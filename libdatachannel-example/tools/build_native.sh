#!/bin/bash
set -euo pipefail

BUILD_DIR=${1:-build}

cmake -S "$(dirname "$0")/.." -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

echo "Native application built in $BUILD_DIR/native_app"

