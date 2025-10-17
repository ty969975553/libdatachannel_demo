#!/bin/bash

# 设置Emscripten环境
source /path/to/emsdk/emsdk_env.sh

# 创建构建目录
mkdir -p build_wasm
cd build_wasm

# 运行CMake以生成Makefile
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/emscripten.toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# 编译WebAssembly应用程序
make

# 返回到原始目录
cd ..