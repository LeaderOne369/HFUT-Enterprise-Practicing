#!/bin/bash

# 现代聊天室测试脚本
# 功能：自动构建项目并启动一个服务器和一个客户端进行测试

set -e  # 遇到错误立即退出

echo "🚀 现代聊天室测试脚本 v1.0"
echo "=============================="

# 定义颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查是否安装了必要的工具
check_dependencies() {
    echo -e "${BLUE}📋 检查依赖项...${NC}"
    
    if ! command -v cmake &> /dev/null; then
        echo -e "${RED}❌ 错误: 未找到 cmake。请先安装 CMake。${NC}"
        exit 1
    fi
    
    if ! command -v make &> /dev/null; then
        echo -e "${RED}❌ 错误: 未找到 make。请确保安装了构建工具。${NC}"
        exit 1
    fi
    
    # 检查Qt
    if ! pkg-config --exists Qt5Core 2>/dev/null && ! pkg-config --exists Qt6Core 2>/dev/null; then
        echo -e "${YELLOW}⚠️  警告: 未检测到 Qt 开发库。请确保已安装 Qt5 或 Qt6。${NC}"
    fi
    
    echo -e "${GREEN}✅ 依赖项检查完成${NC}"
}

# 清理构建目录
clean_build() {
    echo -e "${BLUE}🧹 清理构建目录...${NC}"
    if [ -d "build" ]; then
        rm -rf build
        echo "已删除旧的构建目录"
    fi
}

# 构建项目
build_project() {
    echo -e "${BLUE}🔨 构建项目...${NC}"
    
    # 创建构建目录
    mkdir -p build
    cd build
    
    # 配置项目
    echo "正在配置 CMake..."
    if ! CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake ..; then
        echo -e "${RED}❌ CMake 配置失败${NC}"
        exit 1
    fi
    
    # 编译项目
    echo "正在编译项目..."
    if ! make -j$(nproc 2>/dev/null || echo 4); then
        echo -e "${RED}❌ 编译失败${NC}"
        exit 1
    fi
    
    cd ..
    echo -e "${GREEN}✅ 项目构建成功${NC}"
}

# 检查可执行文件
check_executable() {
    echo -e "${BLUE}🔍 检查可执行文件...${NC}"
    
    # 检查不同平台的可执行文件路径
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        EXECUTABLE="build/final.app/Contents/MacOS/final"
    else
        # Linux
        EXECUTABLE="build/final"
    fi
    
    if [ ! -f "$EXECUTABLE" ]; then
        echo -e "${RED}❌ 错误: 找不到可执行文件 $EXECUTABLE${NC}"
        exit 1
    fi
    
    if [ ! -x "$EXECUTABLE" ]; then
        echo -e "${RED}❌ 错误: 可执行文件没有执行权限${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✅ 可执行文件检查完成: $EXECUTABLE${NC}"
}

# 启动应用实例
start_applications() {
    echo -e "${BLUE}🚀 启动应用实例...${NC}"
    
    echo -e "${YELLOW}📝 使用说明：${NC}"
    echo "即将启动三个聊天应用窗口："
    echo "1. 第一个窗口 - 请选择【服务器模式】，设置昵称（如：服务器管理员），端口 8888"
    echo "2. 第二个窗口 - 请选择【客户端模式】，昵称（如：用户1），连接 127.0.0.1:8888"
    echo "3. 第三个窗口 - 请选择【客户端模式】，昵称（如：用户2），连接 127.0.0.1:8888"
    echo "4. 连接成功后，您可以测试以下功能："
    echo "   ✅ 群聊消息收发（多人同时聊天）"
    echo "   ✅ 点对点文件传输（带实时进度显示）"
    echo "   ✅ SQLite聊天记录存储（菜单 → 聊天 → 查看聊天记录）"
    echo "   ✅ 私聊功能（双击用户列表中的用户名）"
    echo "   ✅ 外观设置（字体、颜色自定义）"
    echo "5. 关闭任一窗口后，脚本将自动清理并退出"
    echo ""
    
    echo -e "${GREEN}🖥️  启动服务器实例...${NC}"
    
    # 启动第一个实例（服务器）
    nohup "$EXECUTABLE" > /dev/null 2>&1 &
    SERVER_PID=$!
    echo -e "${BLUE}   服务器进程 PID: $SERVER_PID${NC}"
    sleep 3  # 等待服务器实例完全启动
    
    echo -e "${GREEN}💻 启动第一个客户端实例...${NC}"
    nohup "$EXECUTABLE" > /dev/null 2>&1 &
    CLIENT1_PID=$!
    echo -e "${BLUE}   客户端1 PID: $CLIENT1_PID${NC}"
    sleep 2  # 等待第一个客户端启动
    
    echo -e "${GREEN}💻 启动第二个客户端实例...${NC}"
    nohup "$EXECUTABLE" > /dev/null 2>&1 &
    CLIENT2_PID=$!
    echo -e "${BLUE}   客户端2 PID: $CLIENT2_PID${NC}"
    sleep 1  # 等待第二个客户端启动
    
    echo ""
    echo -e "${BLUE}🔍 检查进程状态...${NC}"
    
    # 检查所有进程是否正在运行
    running_count=0
    if kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${GREEN}   ✅ 服务器进程运行正常 (PID: $SERVER_PID)${NC}"
        running_count=$((running_count + 1))
    else
        echo -e "${RED}   ❌ 服务器进程未运行${NC}"
    fi
    
    if kill -0 $CLIENT1_PID 2>/dev/null; then
        echo -e "${GREEN}   ✅ 客户端1进程运行正常 (PID: $CLIENT1_PID)${NC}"
        running_count=$((running_count + 1))
    else
        echo -e "${RED}   ❌ 客户端1进程未运行${NC}"
    fi
    
    if kill -0 $CLIENT2_PID 2>/dev/null; then
        echo -e "${GREEN}   ✅ 客户端2进程运行正常 (PID: $CLIENT2_PID)${NC}"
        running_count=$((running_count + 1))
    else
        echo -e "${RED}   ❌ 客户端2进程未运行${NC}"
    fi
    
    echo ""
    if [ $running_count -eq 3 ]; then
        echo -e "${GREEN}🎉 三个应用实例全部启动成功！${NC}"
    else
        echo -e "${YELLOW}⚠️  有 $running_count 个进程启动成功，共需要3个进程${NC}"
    fi
    echo ""
    echo -e "${YELLOW}⏳ 现在请按照上述说明配置连接...${NC}"
    echo -e "${YELLOW}💡 提示：${NC}"
    echo "   • 测试群聊：在任一客户端发送消息，其他窗口都能看到"
    echo "   • 测试私聊：双击用户列表中的用户名发送私聊消息"
    echo "   • 测试文件传输：点击'发送文件'按钮，观察传输进度条"
    echo "   • 测试数据库：发送几条消息后，菜单→聊天→查看聊天记录"
    echo -e "${YELLOW}🔄 关闭任一窗口或按 Ctrl+C 退出测试${NC}"
    echo -e "${YELLOW}📋 或者输入 's' 然后按回车手动停止所有进程${NC}"
    echo ""
    echo -e "${BLUE}📊 进程监控中... (每10秒检查一次状态)${NC}"
    
    # 持续监控进程状态
    while true; do
        # 非阻塞检查用户输入
        if read -t 10 -n 1 user_input 2>/dev/null; then
            if [[ "$user_input" == "s" || "$user_input" == "S" ]]; then
                echo -e "\n${YELLOW}🛑 用户请求停止，正在终止所有进程...${NC}"
                break
            fi
        fi
        
        # 检查是否还有进程在运行
        running_pids=""
        if kill -0 $SERVER_PID 2>/dev/null; then
            running_pids="$running_pids SERVER($SERVER_PID)"
        fi
        if kill -0 $CLIENT1_PID 2>/dev/null; then
            running_pids="$running_pids CLIENT1($CLIENT1_PID)"
        fi
        if kill -0 $CLIENT2_PID 2>/dev/null; then
            running_pids="$running_pids CLIENT2($CLIENT2_PID)"
        fi
        
        if [ -z "$running_pids" ]; then
            echo -e "${YELLOW}📴 所有进程已结束，退出监控${NC}"
            break
        else
            echo -e "${GREEN}📈 运行中的进程:$running_pids${NC}"
        fi
    done
}

# 清理函数
cleanup() {
    echo -e "\n${BLUE}🧹 清理测试环境...${NC}"
    
    # 终止进程
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
    fi
    if [ ! -z "$CLIENT1_PID" ]; then
        kill $CLIENT1_PID 2>/dev/null || true
    fi
    if [ ! -z "$CLIENT2_PID" ]; then
        kill $CLIENT2_PID 2>/dev/null || true
    fi
    
    # 清理聊天应用进程（防止有遗留进程）
    pkill -f "final" 2>/dev/null || true
    
    echo -e "${GREEN}✅ 清理完成${NC}"
    echo -e "${BLUE}👋 测试会话结束${NC}"
}

# 设置信号处理
trap cleanup EXIT INT TERM

# 主执行流程
main() {
    echo -e "${BLUE}开始测试现代聊天室应用...${NC}"
    echo ""
    
    check_dependencies
    clean_build
    build_project
    check_executable
    start_applications
}

# 显示帮助信息
show_help() {
    echo "现代聊天室测试脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help     显示此帮助信息"
    echo "  -c, --clean    仅清理构建目录"
    echo "  -b, --build    仅构建项目"
    echo ""
    echo "功能测试项目:"
    echo "  ✅ TCP 客户端-服务器通信"
    echo "  ✅ 实时聊天消息收发"
    echo "  ✅ 点对点文件传输（带进度显示）"
    echo "  ✅ SQLite 聊天记录存储"
    echo "  ✅ 用户连接管理"
    echo ""
    echo "测试说明:"
    echo "  脚本将自动构建项目并启动一个服务器实例和两个客户端实例"
    echo "  您可以测试群聊、私聊、文件传输、聊天记录存储等功能"
    echo "  关闭任一窗口后脚本将自动退出"
}

# 处理命令行参数
case "${1:-}" in
    -h|--help)
        show_help
        exit 0
        ;;
    -c|--clean)
        clean_build
        exit 0
        ;;
    -b|--build)
        check_dependencies
        build_project
        check_executable
        exit 0
        ;;
    "")
        main
        ;;
    *)
        echo -e "${RED}❌ 未知选项: $1${NC}"
        echo "使用 $0 --help 查看帮助信息"
        exit 1
        ;;
esac 