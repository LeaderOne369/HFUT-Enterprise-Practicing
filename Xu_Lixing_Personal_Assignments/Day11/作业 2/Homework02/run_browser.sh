#!/bin/bash

# 精美浏览器启动脚本
# 解决macOS 15.x上QtWebEngine的GPU兼容性问题

echo "🚀 启动精美浏览器..."
echo "🔧 正在应用macOS 15.x兼容性补丁..."

# 设置环境变量禁用GPU加速，解决崩溃问题
export QTWEBENGINE_DISABLE_GPU=1
export QTWEBENGINE_CHROMIUM_FLAGS="--disable-gpu --disable-gpu-compositing --disable-gpu-rasterization --disable-gpu-sandbox --disable-software-rasterizer --no-sandbox"

# 切换到build目录并运行程序
cd "$(dirname "$0")/build"

if [ -f "Homework02.app/Contents/MacOS/Homework02" ]; then
    echo "✅ 启动浏览器应用程序..."
    ./Homework02.app/Contents/MacOS/Homework02
else
    echo "❌ 错误：找不到可执行文件，请先编译项目"
    echo "💡 运行: cd build && make"
fi 