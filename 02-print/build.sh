#!/bin/bash

# 脚本所在目录（工程根目录）
PROJECT_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$BUILD_DIR/dist"
TOOLS_DIR="$PROJECT_ROOT/../tools"  # 同级目录中的 tools

# 创建 build 目录（如果不存在）
mkdir -p "$BUILD_DIR"

# 确保 tools 目录存在
if [ ! -d "$TOOLS_DIR" ]; then
    echo "错误：找不到 tools 目录 ($TOOLS_DIR)"
    exit 1
fi

# 进入 build 目录执行 CMake
cd "$BUILD_DIR" || exit

# 初始化 CMake（如果未初始化）
if [ ! -f "CMakeCache.txt" ]; then
    cmake ..
fi

# 清理并重新构建
cmake --build . --target clean
cmake --build .

# 检查构建是否成功
if [ ! -f "$DIST_DIR/spl.bin" ]; then
    echo "错误：找不到 $DIST_DIR/spl.bin 文件"
    exit 1
fi

# 生成 idbloader.img
"$TOOLS_DIR/mkimage" -n rk3588 -T rksd -d \
    "$TOOLS_DIR/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin":"$DIST_DIR/spl.bin" \
    "$DIST_DIR/idbloader.img"

echo "构建完成！产物位于: $DIST_DIR/idbloader.img"
