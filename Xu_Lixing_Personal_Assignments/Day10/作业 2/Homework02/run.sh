#!/bin/bash

# 生产者-消费者线程同步演示程序运行脚本
echo "正在构建生产者-消费者线程同步演示程序..."

# 创建构建目录
mkdir -p build
cd build

# 配置和编译
cmake .. && make

if [ $? -eq 0 ]; then
    echo "编译成功！正在启动程序..."
    
    # 在macOS上启动应用
    if [[ "$OSTYPE" == "darwin"* ]]; then
        open Homework02.app
    else
        # 在Linux上启动应用
        ./Homework02
    fi
else
    echo "编译失败，请检查错误信息。"
    exit 1
fi 