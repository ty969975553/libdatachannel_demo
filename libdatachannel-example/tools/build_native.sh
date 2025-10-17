#!/bin/bash

# 设置构建目录
BUILD_DIR=build_native
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 运行CMake以配置构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译项目
cmake --build . --target all

# 输出可执行文件
echo "本地应用程序已成功构建，输出在 $BUILD_DIR 目录中。"