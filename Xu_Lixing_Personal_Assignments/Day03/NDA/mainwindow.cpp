#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QSplitter>
#include <QIcon>
#include <QFileDialog>
#include <QPalette>
#include <QPixmap>
#include <QBrush>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , udpSocket(nullptr)
    , sendTimer(new QTimer(this))
    , isPortBound(false)
    , messageCount(0)
    , totalBytesReceived(0)
    , totalBytesSent(0)
    , isPaused(false)
{
    ui->setupUi(this);
    
    // 创建动作组件
    createActions();
    
    // 设置菜单栏、工具栏、状态栏
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    
    // 设置主界面
    setupUI();
    
    // 设置UI组件样式（背景图通过paintEvent绘制）
    updateBackgroundImage();
    
    // 获取本地IP
    localIP = getLocalIP();
    localIpValue->setText(localIP);
    
    // 设置定时器
    connect(sendTimer, &QTimer::timeout, this, &MainWindow::onTimerSend);
    
    // 设置窗口属性
    setWindowTitle("🌐 网络调试助手 v2.0");
    resize(1000, 800);
    
    // 设置背景色和整体样式
    setStyleSheet(
        "QMainWindow { background-color: #EAE6DA; }"
        "QGroupBox { font-weight: bold; }"
        "QTabWidget::pane { border: 1px solid #C0C0C0; }"
        "QTabBar::tab { background: #F0F0F0; padding: 8px 16px; }"
        "QTabBar::tab:selected { background: #FFFFFF; }"
    );
}

MainWindow::~MainWindow()
{
    if (udpSocket) {
        udpSocket->close();
        delete udpSocket;
    }
    delete ui;
}

void MainWindow::createActions()
{
    // 文件操作动作
    newAction = new QAction("📄 新建会话(&N)", this);
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip("创建新的网络调试会话");
    connect(newAction, &QAction::triggered, this, &MainWindow::newSession);
    
    openAction = new QAction("📂 打开会话(&O)", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("打开已保存的网络调试会话");
    connect(openAction, &QAction::triggered, this, &MainWindow::openSession);
    
    saveAction = new QAction("💾 保存会话(&S)", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("保存当前网络调试会话");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveSession);
    
    exitAction = new QAction("🚪 退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("退出网络调试助手");
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    
    // 连接控制动作
    connectAction = new QAction("🔗 连接(&C)", this);
    connectAction->setStatusTip("连接到网络端口");
    connect(connectAction, &QAction::triggered, this, &MainWindow::connectToHost);
    
    disconnectAction = new QAction("🔌 断开连接(&D)", this);
    disconnectAction->setStatusTip("断开网络连接");
    disconnectAction->setEnabled(false);
    connect(disconnectAction, &QAction::triggered, this, &MainWindow::disconnectFromHost);
    
    pauseAction = new QAction("⏸️ 暂停/恢复(&P)", this);
    pauseAction->setStatusTip("暂停或恢复数据接收");
    pauseAction->setCheckable(true);
    connect(pauseAction, &QAction::triggered, this, &MainWindow::pauseResume);
    
    // 编辑操作动作
    clearSendAction = new QAction("🧹 清空发送区(&E)", this);
    clearSendAction->setStatusTip("清空发送区域内容");
    connect(clearSendAction, &QAction::triggered, this, &MainWindow::clearSendArea);
    
    clearReceiveAction = new QAction("🗑️ 清空接收区(&R)", this);
    clearReceiveAction->setStatusTip("清空接收区域内容");
    connect(clearReceiveAction, &QAction::triggered, this, &MainWindow::clearReceiveArea);
    
    // 工具和帮助动作
    settingsAction = new QAction("⚙️ 设置(&S)", this);
    settingsAction->setStatusTip("打开设置对话框");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
    
    helpAction = new QAction("❓ 帮助(&H)", this);
    helpAction->setStatusTip("查看帮助文档");
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
    
    aboutAction = new QAction("ℹ️ 关于(&A)", this);
    aboutAction->setStatusTip("关于网络调试助手");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    // 连接菜单
    QMenu *connectionMenu = menuBar->addMenu("连接(&C)");
    connectionMenu->addAction(connectAction);
    connectionMenu->addAction(disconnectAction);
    connectionMenu->addSeparator();
    connectionMenu->addAction(pauseAction);
    
    // 编辑菜单
    QMenu *editMenu = menuBar->addMenu("编辑(&E)");
    editMenu->addAction(clearSendAction);
    editMenu->addAction(clearReceiveAction);
    
    // 工具菜单
    QMenu *toolsMenu = menuBar->addMenu("工具(&T)");
    toolsMenu->addAction(settingsAction);
    
    // 帮助菜单
    QMenu *helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction(helpAction);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolBar()
{
    mainToolBar = addToolBar("主工具栏");
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    // 文件操作
    mainToolBar->addAction(newAction);
    mainToolBar->addAction(openAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addSeparator();
    
    // 连接控制
    mainToolBar->addAction(connectAction);
    mainToolBar->addAction(disconnectAction);
    mainToolBar->addAction(pauseAction);
    mainToolBar->addSeparator();
    
    // 编辑操作
    mainToolBar->addAction(clearSendAction);
    mainToolBar->addAction(clearReceiveAction);
    mainToolBar->addSeparator();
    
    // 工具和帮助
    mainToolBar->addAction(settingsAction);
    mainToolBar->addAction(helpAction);
}

void MainWindow::setupStatusBar()
{
    QStatusBar *statusBarWidget = this->statusBar();
    
    // 连接状态标签
    connectionStatusLabel = new QLabel("未连接");
    connectionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    statusBarWidget->addWidget(connectionStatusLabel);
    
    statusBarWidget->addPermanentWidget(new QLabel(" | "));
    
    // 消息计数标签
    messageCountLabel = new QLabel("消息: 0");
    statusBarWidget->addPermanentWidget(messageCountLabel);
    
    statusBarWidget->addPermanentWidget(new QLabel(" | "));
    
    // 字节计数标签
    bytesCountLabel = new QLabel("接收: 0B / 发送: 0B");
    statusBarWidget->addPermanentWidget(bytesCountLabel);
    
    statusBarWidget->addPermanentWidget(new QLabel(" | "));
    
    // 进度条
    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    progressBar->setMaximumWidth(200);
    statusBarWidget->addPermanentWidget(progressBar);
    
    statusBarWidget->showMessage("就绪", 2000);
}

void MainWindow::updateBackgroundImage()
{
    // 设置其他UI组件的透明样式
    this->setStyleSheet(QString(
        "QTabWidget::pane { "
        "    border: 1px solid #C0C0C0; "
        "    background-color: rgba(255, 255, 255, 180); "
        "} "
        "QTabBar::tab { "
        "    background: rgba(240, 240, 240, 180); "
        "    padding: 8px 16px; "
        "    border: 1px solid #C0C0C0; "
        "} "
        "QTabBar::tab:selected { "
        "    background: rgba(255, 255, 255, 200); "
        "} "
        "QGroupBox { "
        "    font-weight: bold; "
        "    background-color: rgba(255, 255, 255, 160); "
        "    border: 1px solid #C0C0C0; "
        "    border-radius: 5px; "
        "    padding-top: 15px; "
        "} "
        "QTextEdit, QLineEdit { "
        "    background-color: rgba(255, 255, 255, 200); "
        "    border: 1px solid #C0C0C0; "
        "} "
        "QPushButton { "
        "    background-color: rgba(240, 240, 240, 180); "
        "    border: 1px solid #C0C0C0; "
        "    padding: 5px; "
        "    border-radius: 3px; "
        "} "
        "QPushButton:hover { "
        "    background-color: rgba(230, 230, 230, 200); "
        "} "
        "QPushButton:pressed { "
        "    background-color: rgba(200, 200, 200, 220); "
        "} "
    ));
}

void MainWindow::setupUI()
{
    // 创建中央组件
    QWidget *centralWidget = new QWidget;
    centralWidget->setObjectName("centralWidget");
    // 背景设置通过paintEvent处理，这里设置为透明
    centralWidget->setStyleSheet("QWidget#centralWidget { background: transparent; }");
    setCentralWidget(centralWidget);
    
    // 创建选项卡
    tabWidget = new QTabWidget;
    udpTab = new QWidget;
    tcpServerTab = new QWidget;
    tcpClientTab = new QWidget;
    
    tabWidget->addTab(udpTab, "UDP测试");
    tabWidget->addTab(tcpServerTab, "TCP服务器");
    tabWidget->addTab(tcpClientTab, "TCP客户端");
    
    // 设置主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(tabWidget);
    
    // 设置UDP选项卡
    setupUdpTab();
    
    // 设置TCP选项卡（基本界面）
    setupTcpTabs();
}

void MainWindow::setupUdpTab()
{
    QHBoxLayout *udpMainLayout = new QHBoxLayout(udpTab);
    
    // 创建分割器
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    udpMainLayout->addWidget(splitter);
    
    // 左侧参数设置区域
    QGroupBox *paramGroup = new QGroupBox;
    paramGroup->setMaximumWidth(280);
    paramGroup->setMinimumWidth(280);
    // 设置垂直大小策略，允许参数组根据内容扩展
    paramGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    
    QVBoxLayout *leftLayout = new QVBoxLayout(paramGroup);
    // 调整布局间距，为提示区域留出更多空间
    leftLayout->setSpacing(8);
    
    // 本地IP
    QHBoxLayout *localIpLayout = new QHBoxLayout;
    localIpLabel = new QLabel("本地IP:");
    localIpValue = new QLabel("172.20.10.5");
    localIpLayout->addWidget(localIpLabel);
    localIpLayout->addWidget(localIpValue);
    localIpLayout->addStretch();
    leftLayout->addLayout(localIpLayout);
    
    // 本地端口
    QHBoxLayout *localPortLayout = new QHBoxLayout;
    localPortLabel = new QLabel("本地端口:");
    localPortEdit = new QLineEdit;
    localPortLayout->addWidget(localPortLabel);
    localPortLayout->addWidget(localPortEdit);
    leftLayout->addLayout(localPortLayout);
    
    // 绑定和关闭按钮
    QHBoxLayout *bindLayout = new QHBoxLayout;
    bindPortBtn = new QPushButton("开始绑定该端口");
    closePortBtn = new QPushButton("关闭端口");
    bindLayout->addWidget(bindPortBtn);
    bindLayout->addWidget(closePortBtn);
    leftLayout->addLayout(bindLayout);
    
    // 状态标签
    statusLabel = new QLabel("未绑定端口");
    statusLabel->setStyleSheet("color: blue;");
    leftLayout->addWidget(statusLabel);
    
    // 目标IP
    QHBoxLayout *targetIpLayout = new QHBoxLayout;
    targetIpLabel = new QLabel("目标IP:");
    targetIpEdit = new QLineEdit;
    targetIpLayout->addWidget(targetIpLabel);
    targetIpLayout->addWidget(targetIpEdit);
    leftLayout->addLayout(targetIpLayout);
    
    // 目标端口
    QHBoxLayout *targetPortLayout = new QHBoxLayout;
    targetPortLabel = new QLabel("目标端口:");
    targetPortEdit = new QLineEdit("8089");
    targetPortLayout->addWidget(targetPortLabel);
    targetPortLayout->addWidget(targetPortEdit);
    leftLayout->addLayout(targetPortLayout);
    
    // 绑定组IP
    QHBoxLayout *groupLayout = new QHBoxLayout;
    groupIpEdit = new QLineEdit;
    bindGroupBtn = new QPushButton("绑定组");
    groupLayout->addWidget(groupIpEdit);
    groupLayout->addWidget(bindGroupBtn);
    leftLayout->addLayout(groupLayout);
    
    // 提示文本
    hintTextEdit = new QTextEdit;
    // 移除固定高度限制，允许高度自适应
    hintTextEdit->setMinimumHeight(80);  // 设置最小高度，确保基本可见
    hintTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // 允许垂直扩展
    // 设置为只读模式，避免用户意外编辑
    hintTextEdit->setReadOnly(true);
    // 设置垂直滚动条策略，当内容超出时显示滚动条
    hintTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    hintTextEdit->setHtml(
        "<p>提示：</p>"
        "<p><b style='color: red;'>多播</b>：使用组播地址(224.0.0.0-239.255.255.255)进行多播通信</p>"
        "<p><b style='color: red;'>广播</b>：使用广播地址进行广播通信，可到达同一网段内的所有主机</p>"
        "<p>UDP是无连接协议，适用于实时通信场景</p>"
    );
    leftLayout->addWidget(hintTextEdit);
    // 移除addStretch()，让提示区域能够自适应扩展
    
    splitter->addWidget(paramGroup);
    
    // 右侧区域
    QWidget *rightWidget = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    
    // 发送区域
    QGroupBox *sendGroup = new QGroupBox("发送区");
    QVBoxLayout *sendLayout = new QVBoxLayout(sendGroup);
    
    // 定时发送设置
    QHBoxLayout *timerLayout = new QHBoxLayout;
    timerSendCheck = new QCheckBox("定时发送");
    QLabel *everyLabel = new QLabel("每隔");
    intervalEdit = new QLineEdit("1000");
    intervalEdit->setMaximumWidth(80);
    QLabel *msLabel = new QLabel("ms");
    timerLayout->addStretch();
    timerLayout->addWidget(timerSendCheck);
    timerLayout->addWidget(everyLabel);
    timerLayout->addWidget(intervalEdit);
    timerLayout->addWidget(msLabel);
    sendLayout->addLayout(timerLayout);
    
    // 广播IP提示
    broadcastIpLabel = new QLabel("IP广播地址为：172.20.10.15");
    broadcastIpLabel->setStyleSheet("color: #FF69B4;");
    sendLayout->addWidget(broadcastIpLabel);
    
    // 发送按钮组
    QHBoxLayout *sendBtnLayout = new QHBoxLayout;
    sendBtn = new QPushButton("发送");
    broadcastSendBtn = new QPushButton("广播发送");
    sendHexBtn = new QPushButton("按16进制发送");
    clearSendBtn = new QPushButton("清空");
    sendBtnLayout->addWidget(sendBtn);
    sendBtnLayout->addWidget(broadcastSendBtn);
    sendBtnLayout->addWidget(sendHexBtn);
    sendBtnLayout->addWidget(clearSendBtn);
    sendLayout->addLayout(sendBtnLayout);
    
    // 发送文本框
    sendTextEdit = new QTextEdit;
    sendTextEdit->setMaximumHeight(120);
    sendLayout->addWidget(sendTextEdit);
    
    rightLayout->addWidget(sendGroup);
    
    // 接收区域
    QGroupBox *receiveGroup = new QGroupBox("接收区");
    QVBoxLayout *receiveLayout = new QVBoxLayout(receiveGroup);
    
    // 接收选项
    QHBoxLayout *receiveOptLayout = new QHBoxLayout;
    hexDisplayCheck = new QCheckBox("按16进制");
    showIpCheck = new QCheckBox("显示IP地址");
    showTimeCheck = new QCheckBox("显示时间");
    showPortCheck = new QCheckBox("显示端口");
    clearReceiveBtn = new QPushButton("清空");
    receiveOptLayout->addWidget(hexDisplayCheck);
    receiveOptLayout->addWidget(showIpCheck);
    receiveOptLayout->addWidget(showTimeCheck);
    receiveOptLayout->addWidget(showPortCheck);
    receiveOptLayout->addStretch();
    receiveOptLayout->addWidget(clearReceiveBtn);
    receiveLayout->addLayout(receiveOptLayout);
    
    // 接收文本框
    receiveTextEdit = new QTextEdit;
    receiveLayout->addWidget(receiveTextEdit);
    
    rightLayout->addWidget(receiveGroup);
    
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    // 连接信号和槽
    connect(bindPortBtn, &QPushButton::clicked, this, &MainWindow::bindPort);
    connect(closePortBtn, &QPushButton::clicked, this, &MainWindow::closePort);
    connect(bindGroupBtn, &QPushButton::clicked, this, &MainWindow::bindGroup);
    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendData);
    connect(broadcastSendBtn, &QPushButton::clicked, this, &MainWindow::broadcastSend);
    connect(sendHexBtn, &QPushButton::clicked, this, &MainWindow::sendHex);
    connect(clearSendBtn, &QPushButton::clicked, this, &MainWindow::clearSendArea);
    connect(clearReceiveBtn, &QPushButton::clicked, this, &MainWindow::clearReceiveArea);
    
    // 定时发送
    connect(timerSendCheck, &QCheckBox::toggled, [this](bool checked) {
        if (checked && isPortBound) {
            int interval = intervalEdit->text().toInt();
            if (interval > 0) {
                sendTimer->start(interval);
            }
        } else {
            sendTimer->stop();
        }
    });
}

QString MainWindow::getLocalIP()
{
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && 
            !address.isLoopback() && 
            address != QHostAddress::LocalHost) {
            return address.toString();
        }
    }
    return "127.0.0.1";
}

void MainWindow::bindPort()
{
    if (isPortBound) {
        QMessageBox::warning(this, "警告", "端口已经绑定，请先关闭端口");
        return;
    }
    
    bool ok;
    int port = localPortEdit->text().toInt(&ok);
    if (!ok || port < 1 || port > 65535) {
        QMessageBox::warning(this, "错误", "请输入有效的端口号(1-65535)");
        return;
    }
    
    if (!udpSocket) {
        udpSocket = new QUdpSocket(this);
        connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::onUdpReceiveData);
    }
    
    if (udpSocket->bind(QHostAddress::Any, port)) {
        isPortBound = true;
        statusLabel->setText(QString("已绑定端口：%1").arg(port));
        statusLabel->setStyleSheet("color: green;");
        bindPortBtn->setEnabled(false);
        closePortBtn->setEnabled(true);
        
        // 更新状态栏和工具栏
        connectionStatusLabel->setText("已连接");
        connectionStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        connectAction->setEnabled(false);
        disconnectAction->setEnabled(true);
        this->statusBar()->showMessage(QString("成功绑定端口 %1").arg(port), 3000);
        
        // 更新广播地址
        QString broadcastAddr = localIP;
        QStringList parts = broadcastAddr.split('.');
        if (parts.size() == 4) {
            parts[3] = "255";
            broadcastAddr = parts.join('.');
            broadcastIpLabel->setText(QString("IP广播地址为：%1").arg(broadcastAddr));
        }
    } else {
        QMessageBox::critical(this, "错误", "端口绑定失败：" + udpSocket->errorString());
        this->statusBar()->showMessage("端口绑定失败", 3000);
    }
}

void MainWindow::closePort()
{
    if (udpSocket && isPortBound) {
        udpSocket->close();
        isPortBound = false;
        statusLabel->setText("未绑定端口");
        statusLabel->setStyleSheet("color: blue;");
        bindPortBtn->setEnabled(true);
        closePortBtn->setEnabled(false);
        sendTimer->stop();
        timerSendCheck->setChecked(false);
        
        // 更新状态栏和工具栏
        connectionStatusLabel->setText("未连接");
        connectionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
        connectAction->setEnabled(true);
        disconnectAction->setEnabled(false);
        pauseAction->setChecked(false);
        isPaused = false;
        this->statusBar()->showMessage("连接已断开", 3000);
    }
}

void MainWindow::bindGroup()
{
    QString groupAddr = groupIpEdit->text().trimmed();
    if (groupAddr.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入组播地址");
        return;
    }
    
    QHostAddress groupAddress(groupAddr);
    if (!groupAddress.isMulticast()) {
        QMessageBox::warning(this, "警告", "请输入有效的组播地址(224.0.0.0-239.255.255.255)");
        return;
    }
    
    if (udpSocket && isPortBound) {
        if (udpSocket->joinMulticastGroup(groupAddress)) {
            receiveTextEdit->append(QString("[%1] 成功加入组播组：%2")
                                  .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                  .arg(groupAddr));
        } else {
            QMessageBox::critical(this, "错误", "加入组播组失败：" + udpSocket->errorString());
        }
    } else {
        QMessageBox::warning(this, "警告", "请先绑定端口");
    }
}

void MainWindow::sendData()
{
    if (!udpSocket || !isPortBound) {
        QMessageBox::warning(this, "警告", "请先绑定端口");
        return;
    }
    
    QString targetIp = targetIpEdit->text().trimmed();
    bool ok;
    int targetPort = targetPortEdit->text().toInt(&ok);
    
    if (targetIp.isEmpty() || !ok || targetPort < 1 || targetPort > 65535) {
        QMessageBox::warning(this, "错误", "请输入有效的目标IP和端口");
        return;
    }
    
    QString data = sendTextEdit->toPlainText();
    if (data.isEmpty()) {
        QMessageBox::warning(this, "警告", "发送内容不能为空");
        return;
    }
    
    QByteArray sendData = data.toUtf8();
    qint64 written = udpSocket->writeDatagram(sendData, QHostAddress(targetIp), targetPort);
    
    if (written == -1) {
        QMessageBox::critical(this, "错误", "发送失败：" + udpSocket->errorString());
        this->statusBar()->showMessage("发送失败", 2000);
    } else {
        totalBytesSent += written;
        bytesCountLabel->setText(QString("接收: %1B / 发送: %2B")
                               .arg(totalBytesReceived)
                               .arg(totalBytesSent));
        
        receiveTextEdit->append(QString("[%1] 发送到 %2:%3 - %4")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(targetIp)
                              .arg(targetPort)
                              .arg(data));
        this->statusBar()->showMessage(QString("发送 %1 字节").arg(written), 2000);
    }
}

void MainWindow::broadcastSend()
{
    if (!udpSocket || !isPortBound) {
        QMessageBox::warning(this, "警告", "请先绑定端口");
        return;
    }
    
    QString data = sendTextEdit->toPlainText();
    if (data.isEmpty()) {
        QMessageBox::warning(this, "警告", "发送内容不能为空");
        return;
    }
    
    // 计算广播地址
    QString broadcastAddr = localIP;
    QStringList parts = broadcastAddr.split('.');
    if (parts.size() == 4) {
        parts[3] = "255";
        broadcastAddr = parts.join('.');
    }
    
    bool ok;
    int targetPort = targetPortEdit->text().toInt(&ok);
    if (!ok || targetPort < 1 || targetPort > 65535) {
        targetPort = 8089;
    }
    
    QByteArray sendData = data.toUtf8();
    qint64 written = udpSocket->writeDatagram(sendData, QHostAddress(broadcastAddr), targetPort);
    
    if (written == -1) {
        QMessageBox::critical(this, "错误", "广播发送失败：" + udpSocket->errorString());
    } else {
        receiveTextEdit->append(QString("[%1] 广播发送到 %2:%3 - %4")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(broadcastAddr)
                              .arg(targetPort)
                              .arg(data));
    }
}

void MainWindow::sendHex()
{
    if (!udpSocket || !isPortBound) {
        QMessageBox::warning(this, "警告", "请先绑定端口");
        return;
    }
    
    QString targetIp = targetIpEdit->text().trimmed();
    bool ok;
    int targetPort = targetPortEdit->text().toInt(&ok);
    
    if (targetIp.isEmpty() || !ok || targetPort < 1 || targetPort > 65535) {
        QMessageBox::warning(this, "错误", "请输入有效的目标IP和端口");
        return;
    }
    
    QString hexData = sendTextEdit->toPlainText().replace(" ", "");
    if (hexData.isEmpty()) {
        QMessageBox::warning(this, "警告", "发送内容不能为空");
        return;
    }
    
    QByteArray sendData = QByteArray::fromHex(hexData.toUtf8());
    qint64 written = udpSocket->writeDatagram(sendData, QHostAddress(targetIp), targetPort);
    
    if (written == -1) {
        QMessageBox::critical(this, "错误", "16进制发送失败：" + udpSocket->errorString());
    } else {
        receiveTextEdit->append(QString("[%1] 16进制发送到 %2:%3 - %4")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(targetIp)
                              .arg(targetPort)
                              .arg(hexData));
    }
}

void MainWindow::clearSendArea()
{
    sendTextEdit->clear();
}

void MainWindow::clearReceiveArea()
{
    receiveTextEdit->clear();
}

void MainWindow::onTimerSend()
{
    if (isPortBound && !sendTextEdit->toPlainText().isEmpty()) {
        sendData();
    }
}

void MainWindow::onUdpReceiveData()
{
    if (isPaused) {
        return; // 如果暂停了，不处理数据
    }
    
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        
        QHostAddress sender;
        quint16 senderPort;
        
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        
        // 更新统计信息
        messageCount++;
        totalBytesReceived += datagram.size();
        messageCountLabel->setText(QString("消息: %1").arg(messageCount));
        bytesCountLabel->setText(QString("接收: %1B / 发送: %2B")
                               .arg(totalBytesReceived)
                               .arg(totalBytesSent));
        
        QString displayText;
        QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
        
        // 构建显示文本
        QStringList infoParts;
        if (showTimeCheck->isChecked()) {
            infoParts << QString("[%1]").arg(timeStr);
        }
        if (showIpCheck->isChecked()) {
            infoParts << QString("来自:%1").arg(sender.toString());
        }
        if (showPortCheck->isChecked()) {
            infoParts << QString("端口:%1").arg(senderPort);
        }
        
        QString prefix = infoParts.join(" ");
        if (!prefix.isEmpty()) {
            prefix += " - ";
        }
        
        if (hexDisplayCheck->isChecked()) {
            displayText = prefix + datagram.toHex(' ').toUpper();
        } else {
            displayText = prefix + QString::fromUtf8(datagram);
        }
        
        receiveTextEdit->append(displayText);
        this->statusBar()->showMessage(QString("接收到 %1 字节").arg(datagram.size()), 1000);
    }
}

void MainWindow::setupTcpTabs()
{
    // TCP服务器选项卡
    QVBoxLayout *tcpServerLayout = new QVBoxLayout(tcpServerTab);
    QLabel *tcpServerLabel = new QLabel("TCP服务器功能 - 待实现");
    tcpServerLabel->setAlignment(Qt::AlignCenter);
    tcpServerLabel->setStyleSheet("font-size: 16px; color: #666666; padding: 50px;");
    tcpServerLayout->addWidget(tcpServerLabel);
    
    // TCP客户端选项卡
    QVBoxLayout *tcpClientLayout = new QVBoxLayout(tcpClientTab);
    QLabel *tcpClientLabel = new QLabel("TCP客户端功能 - 待实现");
    tcpClientLabel->setAlignment(Qt::AlignCenter);
    tcpClientLabel->setStyleSheet("font-size: 16px; color: #666666; padding: 50px;");
    tcpClientLayout->addWidget(tcpClientLabel);
}

// 菜单栏和工具栏功能实现
void MainWindow::newSession()
{
    // 清空所有数据
    clearSendArea();
    clearReceiveArea();
    
    // 重置统计数据
    messageCount = 0;
    totalBytesReceived = 0;
    totalBytesSent = 0;
    messageCountLabel->setText("消息: 0");
    bytesCountLabel->setText("接收: 0B / 发送: 0B");
    
    // 重置网络连接
    if (isPortBound) {
        closePort();
    }
    
    this->statusBar()->showMessage("新建会话成功", 2000);
}

void MainWindow::openSession()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "打开会话文件", "", "会话文件 (*.nda);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        // 这里可以实现会话文件的读取功能
        QMessageBox::information(this, "提示", "会话文件读取功能待实现");
        this->statusBar()->showMessage("会话文件读取功能待实现", 3000);
    }
}

void MainWindow::saveSession()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存会话文件", "", "会话文件 (*.nda);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        // 这里可以实现会话文件的保存功能
        QMessageBox::information(this, "提示", "会话文件保存功能待实现");
        this->statusBar()->showMessage("会话文件保存功能待实现", 3000);
    }
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于网络调试助手",
        "<h3>网络调试助手 v2.0</h3>"
        "<p>一个功能强大的网络调试工具，支持UDP/TCP通信测试。</p>"
        "<p><b>主要功能：</b></p>"
        "<ul>"
        "<li>UDP单播、组播、广播通信</li>"
        "<li>TCP服务器和客户端模式</li>"
        "<li>16进制数据发送和接收</li>"
        "<li>定时发送功能</li>"
        "<li>数据统计和监控</li>"
        "</ul>"
        "<p><b>版权所有：</b> 网络调试助手开发团队</p>"
        "<p><b>技术支持：</b> Qt Framework</p>");
}

void MainWindow::showHelp()
{
    QMessageBox::information(this, "帮助",
        "<h3>使用说明</h3>"
        "<p><b>UDP测试：</b></p>"
        "<ol>"
        "<li>在本地端口输入框中输入要绑定的端口号</li>"
        "<li>点击'开始绑定该端口'按钮绑定端口</li>"
        "<li>在目标IP和端口中输入要发送数据的目标地址</li>"
        "<li>在发送区域输入要发送的数据</li>"
        "<li>点击'发送'、'广播发送'或'按16进制发送'按钮发送数据</li>"
        "</ol>"
        "<p><b>组播功能：</b></p>"
        "<p>在组播IP输入框中输入组播地址(224.0.0.0-239.255.255.255)，然后点击'绑定组'按钮。</p>"
        "<p><b>快捷键：</b></p>"
        "<ul>"
        "<li>Ctrl+N: 新建会话</li>"
        "<li>Ctrl+O: 打开会话</li>"
        "<li>Ctrl+S: 保存会话</li>"
        "<li>Ctrl+Q: 退出程序</li>"
        "</ul>");
}

void MainWindow::showSettings()
{
    QMessageBox::information(this, "设置", "设置对话框功能待实现");
}

void MainWindow::connectToHost()
{
    // 这个功能直接调用绑定端口
    bindPort();
}

void MainWindow::disconnectFromHost()
{
    // 这个功能直接调用关闭端口
    closePort();
}

void MainWindow::pauseResume()
{
    isPaused = !isPaused;
    
    if (isPaused) {
        this->statusBar()->showMessage("数据接收已暂停", 3000);
    } else {
        this->statusBar()->showMessage("数据接收已恢复", 3000);
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    
    QPainter painter(this);
    
    // 加载背景图片
    QPixmap background(":/background.png");
    if (!background.isNull()) {
        // 缩放背景图片以铺满整个窗口
        QPixmap scaledBackground = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        
        // 绘制背景图片
        painter.drawPixmap(0, 0, scaledBackground);
    }
}
