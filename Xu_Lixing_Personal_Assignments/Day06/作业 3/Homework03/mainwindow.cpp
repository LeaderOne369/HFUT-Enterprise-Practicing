#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QStandardPaths>
#include <QKeyEvent>
#include <QDate>
#include <QTime>
#include <QRandomGenerator>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , server(nullptr)
    , client(nullptr)
    , fileTransfer(nullptr)
    , database(nullptr)
    , isServerMode(false)
    , currentFileReceiver(nullptr)
{
    ui->setupUi(this);
    
    // 初始化数据库
    database = new Database(this);
    if (!database->initialize()) {
        QMessageBox::warning(this, "警告", "无法初始化数据库，聊天记录功能将不可用");
    }
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    applyStyles();
    connectSignals();
    
    // 设置窗口属性
    setWindowTitle("现代聊天室");
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    // 初始化消息格式
    currentMessageFormat.setForeground(QColor(Qt::black));
    currentMessageFormat.setFont(QFont("微软雅黑", 10));
    
    // 显示连接对话框
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MainWindow::showConnectDialog);
    connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    timer->start(100);
}

MainWindow::~MainWindow()
{
    if (server) {
        server->stopServer();
    }
    if (client) {
        client->disconnectFromServer();
    }
    delete ui;
}

void MainWindow::setupUI()
{
    // 创建主要布局
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 主分割器 - 左右分割
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧 - 在线用户列表
    userListWidget = new QListWidget();
    userListWidget->setMaximumWidth(200);
    userListWidget->setMinimumWidth(150);
    
    // 右侧 - 选项卡控件
    rightTabWidget = new QTabWidget();
    
    // 聊天选项卡
    QWidget* chatTab = new QWidget();
    QVBoxLayout* chatLayout = new QVBoxLayout(chatTab);
    
    // 聊天显示区域
    chatDisplay = new QTextEdit();
    chatDisplay->setReadOnly(true);
    chatDisplay->setMinimumHeight(300);
    
    // 输入区域
    QWidget* inputWidget = new QWidget();
    QVBoxLayout* inputLayout = new QVBoxLayout(inputWidget);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    
    // 工具按钮行
    QWidget* toolWidget = new QWidget();
    QHBoxLayout* toolLayout = new QHBoxLayout(toolWidget);
    toolLayout->setContentsMargins(0, 0, 0, 0);
    
    fontButton = new QPushButton("字体");
    colorButton = new QPushButton("颜色");
    fileButton = new QPushButton("发送文件");
    
    toolLayout->addWidget(fontButton);
    toolLayout->addWidget(colorButton);
    toolLayout->addWidget(fileButton);
    toolLayout->addStretch();
    
    // 消息输入框
    messageInput = new QTextEdit();
    messageInput->setMaximumHeight(80);
    messageInput->setMinimumHeight(60);
    messageInput->setPlaceholderText("在这里输入消息... (支持emoji 😊) (Ctrl+Enter 发送)");
    
    // 发送按钮
    sendButton = new QPushButton("发送 (Ctrl+Enter)");
    sendButton->setDefault(true);
    
    inputLayout->addWidget(toolWidget);
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);
    
    chatLayout->addWidget(chatDisplay);
    chatLayout->addWidget(inputWidget);
    
    // 文件传输进度条
    transferProgressBar = new QProgressBar();
    transferProgressBar->setVisible(false);
    chatLayout->addWidget(transferProgressBar);
    
    rightTabWidget->addTab(chatTab, "聊天");
    
    // 添加到主分割器
    mainSplitter->addWidget(userListWidget);
    mainSplitter->addWidget(rightTabWidget);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    
    // 设置主布局
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(mainSplitter);
    mainLayout->setContentsMargins(5, 5, 5, 5);
}

void MainWindow::setupMenuBar()
{
    // 连接菜单
    QMenu* connectMenu = menuBar()->addMenu("连接");
    connectMenu->addAction("连接到服务器", this, &MainWindow::showConnectDialog);
    connectMenu->addSeparator();
    connectMenu->addAction("断开连接", this, &MainWindow::onDisconnected);
    connectMenu->addAction("退出", this, &QWidget::close);
    
    // 聊天菜单
    QMenu* chatMenu = menuBar()->addMenu("聊天");
    chatMenu->addAction("更改昵称", this, &MainWindow::changeNickname);
    chatMenu->addAction("发送文件", this, &MainWindow::sendFile);
    chatMenu->addSeparator();
    chatMenu->addAction("查看聊天记录", this, &MainWindow::showChatHistory);
    chatMenu->addAction("清空聊天记录", this, &MainWindow::clearChatHistory);
    
    // 外观菜单
    QMenu* appearanceMenu = menuBar()->addMenu("外观");
    appearanceMenu->addAction("更改字体", this, &MainWindow::changeFont);
    appearanceMenu->addAction("更改文字颜色", this, &MainWindow::changeTextColor);
    
    // 帮助菜单
    QMenu* helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction("关于", this, &MainWindow::showAbout);
}

void MainWindow::setupStatusBar()
{
    statusLabel = new QLabel("未连接");
    statusBar()->addWidget(statusLabel);
    statusBar()->addPermanentWidget(new QLabel("现代聊天室 v1.0"));
}

void MainWindow::connectSignals()
{
    // UI信号
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(fileButton, &QPushButton::clicked, this, &MainWindow::sendFile);
    connect(fontButton, &QPushButton::clicked, this, &MainWindow::changeFont);
    connect(colorButton, &QPushButton::clicked, this, &MainWindow::changeTextColor);
    connect(userListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onUserListDoubleClicked);
    
    // 安装事件过滤器到消息输入框
    messageInput->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) 
            && keyEvent->modifiers() == Qt::ControlModifier) {
            sendMessage();
            return true; // 事件已处理
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::applyStyles()
{
    // iOS风格的现代化样式表
    QString iosStyleSheet = R"(
        /* 主窗口背景 */
        QMainWindow {
            background-color: #f2f2f7;
            color: #1c1c1e;
        }
        
        /* 分割器样式 */
        QSplitter::handle {
            background-color: #c6c6c8;
            width: 1px;
            height: 1px;
        }
        
        QSplitter::handle:horizontal {
            width: 1px;
        }
        
        QSplitter::handle:vertical {
            height: 1px;
        }
        
        /* 用户列表样式 */
        QListWidget {
            background-color: #ffffff;
            border: 2px solid #e5e5ea;
            border-radius: 12px;
            padding: 8px;
            font-size: 16px;
            font-weight: 500;
            color: #1c1c1e;
            selection-background-color: #007aff;
            selection-color: white;
            outline: none;
        }
        
        QListWidget::item {
            background-color: transparent;
            border: none;
            border-radius: 8px;
            padding: 12px 16px;
            margin: 2px 0px;
            color: #1c1c1e;
        }
        
        QListWidget::item:hover {
            background-color: #f2f2f7;
        }
        
        QListWidget::item:selected {
            background-color: #007aff;
            color: white;
        }
        
        /* 选项卡样式 */
        QTabWidget::pane {
            border: 2px solid #e5e5ea;
            background-color: #ffffff;
            border-radius: 12px;
            margin-top: 8px;
        }
        
        QTabBar::tab {
            background-color: #f2f2f7;
            color: #8e8e93;
            border: none;
            padding: 12px 24px;
            margin-right: 2px;
            font-size: 16px;
            font-weight: 600;
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
        }
        
        QTabBar::tab:selected {
            background-color: #ffffff;
            color: #007aff;
        }
        
        QTabBar::tab:hover:!selected {
            background-color: #e5e5ea;
        }
        
        /* 聊天显示区域 */
        QTextEdit {
            background-color: #ffffff;
            border: 2px solid #e5e5ea;
            border-radius: 12px;
            padding: 16px;
            font-size: 16px;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
            color: #1c1c1e;
            selection-background-color: #007aff;
            selection-color: white;
        }
        
        /* 消息输入框特殊样式 */
        QTextEdit[objectName="messageInput"] {
            background-color: #f2f2f7;
            border: 2px solid #e5e5ea;
            border-radius: 20px;
            padding: 12px 16px;
            font-size: 16px;
            max-height: 80px;
            min-height: 44px;
        }
        
        QTextEdit[objectName="messageInput"]:focus {
            border-color: #007aff;
            background-color: #ffffff;
        }
        
        /* 按钮样式 */
        QPushButton {
            background-color: #007aff;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 12px 24px;
            font-size: 16px;
            font-weight: 600;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
        }
        
        QPushButton:hover {
            background-color: #0056b3;
        }
        
        QPushButton:pressed {
            background-color: #004494;
            transform: scale(0.98);
        }
        
        QPushButton:disabled {
            background-color: #c6c6c8;
            color: #8e8e93;
        }
        
        /* 工具按钮样式 */
        QPushButton[objectName="fontButton"],
        QPushButton[objectName="colorButton"],
        QPushButton[objectName="fileButton"] {
            background-color: #f2f2f7;
            color: #007aff;
            border: 1px solid #e5e5ea;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: 500;
        }
        
        QPushButton[objectName="fontButton"]:hover,
        QPushButton[objectName="colorButton"]:hover,
        QPushButton[objectName="fileButton"]:hover {
            background-color: #e5e5ea;
        }
        
        /* 发送按钮特殊样式 */
        QPushButton[objectName="sendButton"] {
            background-color: #007aff;
            color: white;
            border-radius: 20px;
            padding: 12px 20px;
            font-size: 16px;
            font-weight: 600;
            min-width: 80px;
        }
        
        /* 进度条样式 */
        QProgressBar {
            background-color: #e5e5ea;
            border: none;
            border-radius: 6px;
            text-align: center;
            font-size: 14px;
            font-weight: 500;
            color: #1c1c1e;
        }
        
        QProgressBar::chunk {
            background-color: #007aff;
            border-radius: 6px;
        }
        
        /* 菜单栏样式 */
        QMenuBar {
            background-color: #f2f2f7;
            border: none;
            font-size: 16px;
            font-weight: 500;
            color: #1c1c1e;
            padding: 4px;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
            border-radius: 8px;
        }
        
        QMenuBar::item:selected {
            background-color: #e5e5ea;
        }
        
        QMenu {
            background-color: #ffffff;
            border: 1px solid #e5e5ea;
            border-radius: 12px;
            padding: 8px;
            font-size: 16px;
            color: #1c1c1e;
        }
        
        QMenu::item {
            background-color: transparent;
            padding: 12px 16px;
            border-radius: 8px;
            margin: 2px;
        }
        
        QMenu::item:selected {
            background-color: #f2f2f7;
        }
        
        QMenu::separator {
            height: 1px;
            background-color: #e5e5ea;
            margin: 8px 16px;
        }
        
        /* 状态栏样式 */
        QStatusBar {
            background-color: #f2f2f7;
            border: none;
            font-size: 14px;
            color: #8e8e93;
        }
        
        QStatusBar QLabel {
            color: #8e8e93;
            font-weight: 500;
        }
        
        /* 滚动条样式 */
        QScrollBar:vertical {
            background-color: #f2f2f7;
            width: 8px;
            border-radius: 4px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #c6c6c8;
            border-radius: 4px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #a6a6a8;
        }
        
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            border: none;
            background: none;
        }
        
        QScrollBar:horizontal {
            background-color: #f2f2f7;
            height: 8px;
            border-radius: 4px;
        }
        
        QScrollBar::handle:horizontal {
            background-color: #c6c6c8;
            border-radius: 4px;
            min-width: 20px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background-color: #a6a6a8;
        }
        
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            border: none;
            background: none;
        }
    )";
    
    setStyleSheet(iosStyleSheet);
    
    // 设置对象名称以便样式表选择器工作
    messageInput->setObjectName("messageInput");
    sendButton->setObjectName("sendButton");
    fontButton->setObjectName("fontButton");
    colorButton->setObjectName("colorButton");
    fileButton->setObjectName("fileButton");
    
    // 设置窗口属性
    setAttribute(Qt::WA_TranslucentBackground, false);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
}

void MainWindow::showConnectDialog()
{
    ConnectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString nickname = dialog.getNickname();
        if (nickname.isEmpty()) {
            QMessageBox::warning(this, "警告", "昵称不能为空");
            return;
        }
        
        currentNickname = nickname;
        isServerMode = dialog.isServerMode();
        
        if (isServerMode) {
            // 服务器模式
            server = new ChatServer(this);
            connect(server, &ChatServer::clientConnected, this, &MainWindow::onClientConnected);
            connect(server, &ChatServer::clientDisconnected, this, &MainWindow::onClientDisconnected);
            connect(server, &ChatServer::messageReceived, this, &MainWindow::onServerMessageReceived);
            
            if (server->startServer(dialog.getServerPort())) {
                statusLabel->setText(QString("服务器运行中 - 端口: %1").arg(dialog.getServerPort()));
                addMessageToChat("系统", QString("服务器已启动，端口: %1").arg(dialog.getServerPort()), 
                               QDateTime::currentDateTime(), false, true);
                
                // 服务器模式下显示已连接客户端
                userListWidget->clear();
                userListWidget->addItem("服务器");
            } else {
                QMessageBox::critical(this, "错误", "无法启动服务器");
                delete server;
                server = nullptr;
            }
        } else {
            // 客户端模式
            client = new ChatClient(this);
            connect(client, &ChatClient::connected, this, &MainWindow::onConnected);
            connect(client, &ChatClient::disconnected, this, &MainWindow::onDisconnected);
            connect(client, &ChatClient::connectionError, this, &MainWindow::onConnectionError);
            connect(client, &ChatClient::messageReceived, this, &MainWindow::onMessageReceived);
            connect(client, &ChatClient::privateMessageReceived, this, &MainWindow::onPrivateMessageReceived);
            connect(client, &ChatClient::systemMessageReceived, this, &MainWindow::onSystemMessageReceived);
            connect(client, &ChatClient::userJoined, this, &MainWindow::onUserJoined);
            connect(client, &ChatClient::userLeft, this, &MainWindow::onUserLeft);
            connect(client, &ChatClient::onlineUsersUpdated, this, &MainWindow::onOnlineUsersUpdated);
            connect(client, &ChatClient::fileTransferRequested, this, &MainWindow::onFileTransferRequested);
            connect(client, &ChatClient::fileTransferResponse, this, &MainWindow::onFileTransferResponse);
            
            statusLabel->setText("连接中...");
            if (!client->connectToServer(dialog.getServerHost(), dialog.getServerPort(), nickname)) {
                QMessageBox::critical(this, "错误", "无法连接到服务器");
                delete client;
                client = nullptr;
                statusLabel->setText("未连接");
            }
        }
    }
}

void MainWindow::onConnected()
{
    statusLabel->setText(QString("已连接 - %1").arg(currentNickname));
    addMessageToChat("系统", "欢迎加入聊天室！", QDateTime::currentDateTime(), false, true);
    
    // 启用UI控件
    sendButton->setEnabled(true);
    fileButton->setEnabled(true);
    messageInput->setEnabled(true);
    
    // 加载聊天历史
    loadChatHistory();
}

void MainWindow::onDisconnected()
{
    if (client) {
        client->deleteLater();
        client = nullptr;
    }
    if (server) {
        server->stopServer();
        server->deleteLater();
        server = nullptr;
    }
    
    statusLabel->setText("未连接");
    addMessageToChat("系统", "连接已断开", QDateTime::currentDateTime(), false, true);
    
    // 禁用UI控件
    sendButton->setEnabled(false);
    fileButton->setEnabled(false);
    messageInput->setEnabled(false);
    
    // 清空用户列表
    userListWidget->clear();
}

void MainWindow::onConnectionError(const QString &error)
{
    statusLabel->setText("连接失败");
    QMessageBox::critical(this, "连接错误", error);
    
    if (client) {
        client->deleteLater();
        client = nullptr;
    }
}

void MainWindow::onMessageReceived(const QString &sender, const QString &message, const QDateTime &timestamp)
{
    // 检查消息是否包含HTML格式
    if (message.contains("<span style=")) {
        addFormattedMessageToChat(sender, message, timestamp);
    } else {
        addMessageToChat(sender, message, timestamp);
    }
    
    // 存储时去掉HTML标签
    QString plainMessage = QTextDocumentFragment::fromHtml(message).toPlainText();
    saveMessageToDatabase(sender, plainMessage);
}

void MainWindow::onPrivateMessageReceived(const QString &sender, const QString &message, const QDateTime &timestamp)
{
    // 在主窗口显示简要信息
    QString privateMsgDisplay = QString("[私聊来自 %1] %2").arg(sender).arg(message);
    addMessageToChat("🔒私聊", privateMsgDisplay, timestamp, true);
    saveMessageToDatabase(sender, QString("[私聊] %1").arg(message));
    
    // 如果有对应的私聊窗口，在私聊窗口中显示
    if (privateChatWindows.contains(sender)) {
        // 检查消息是否包含HTML格式
        QString displayMessage = message;
        if (message.contains("<span style=")) {
            displayMessage = QTextDocumentFragment::fromHtml(message).toPlainText();
        }
        privateChatWindows[sender]->addMessage(sender, displayMessage, timestamp);
    } else {
        // 如果私聊窗口不存在，创建一个但不立即显示
        // 这样可以保存接收到的消息，用户打开窗口时能看到
        PrivateChatWindow *privateChatWindow = new PrivateChatWindow(sender, currentNickname, client, this);
        privateChatWindows[sender] = privateChatWindow;
        
        // 添加消息到新创建的私聊窗口
        QString displayMessage = message;
        if (message.contains("<span style=")) {
            displayMessage = QTextDocumentFragment::fromHtml(message).toPlainText();
        }
        privateChatWindow->addMessage(sender, displayMessage, timestamp);
        
        // 当私聊窗口关闭时从映射中移除
        connect(privateChatWindow, &QDialog::finished, [this, sender]() {
            privateChatWindows.remove(sender);
        });
        
        // 显示有新私聊消息的提醒
        updateUserListWithPrivateMessageIndicator(sender);
    }
    
    // 在状态栏显示私聊消息提醒
    statusLabel->setText(QString("收到来自 %1 的私聊消息").arg(sender));
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, timer]() {
        if (client && client->isConnected()) {
            statusLabel->setText(QString("已连接 - %1").arg(currentNickname));
        } else if (server) {
            statusLabel->setText(QString("服务器运行中 - %1").arg(currentNickname));
        }
        timer->deleteLater();
    });
    timer->start(5000);
}

void MainWindow::onSystemMessageReceived(const QString &message, const QDateTime &timestamp)
{
    addMessageToChat("系统", message, timestamp, false, true);
    saveMessageToDatabase("系统", message);
}

void MainWindow::sendMessage()
{
    QString message = messageInput->toPlainText().trimmed();
    if (message.isEmpty()) return;
    
    // 获取当前格式信息
    QTextCharFormat format = messageInput->currentCharFormat();
    QString colorName = format.foreground().color().name();
    QString fontFamily = format.font().family();
    int fontSize = format.font().pointSize();
    bool bold = format.font().bold();
    bool italic = format.font().italic();
    
    // 构建带格式的HTML消息
    QString formattedMessage = QString("<span style='color:%1; font-family:%2; font-size:%3pt; %4%5'>%6</span>")
                              .arg(colorName)
                              .arg(fontFamily)
                              .arg(fontSize)
                              .arg(bold ? "font-weight:bold;" : "")
                              .arg(italic ? "font-style:italic;" : "")
                              .arg(message.toHtmlEscaped());
    
    if (client && client->isConnected()) {
        client->sendMessage(formattedMessage);
        addFormattedMessageToChat(currentNickname, formattedMessage, QDateTime::currentDateTime());
        saveMessageToDatabase(currentNickname, message);
        messageInput->clear();
    } else if (server) {
        server->sendMessageToAll(formattedMessage, currentNickname);
        addFormattedMessageToChat(currentNickname, formattedMessage, QDateTime::currentDateTime());
        saveMessageToDatabase(currentNickname, message);
        messageInput->clear();
    } else {
        QMessageBox::warning(this, "警告", "未连接到服务器");
    }
}

void MainWindow::onUserJoined(const QString &nickname)
{
    addMessageToChat("系统", QString("%1 加入了聊天室").arg(nickname), QDateTime::currentDateTime(), false, true);
}

void MainWindow::onUserLeft(const QString &nickname)
{
    addMessageToChat("系统", QString("%1 离开了聊天室").arg(nickname), QDateTime::currentDateTime(), false, true);
}

void MainWindow::onOnlineUsersUpdated(const QStringList &users)
{
    updateUserList(users);
}

void MainWindow::onUserListDoubleClicked()
{
    QListWidgetItem* item = userListWidget->currentItem();
    if (!item || !client || !client->isConnected()) return;
    
    QString selectedUser = item->text();
    
    // 移除私聊消息指示器
    if (selectedUser.startsWith("🔴 ")) {
        selectedUser = selectedUser.mid(3); // 移除 "🔴 " 前缀
        item->setText(selectedUser); // 更新显示文本
        item->setToolTip(""); // 清除工具提示
    }
    
    if (selectedUser == currentNickname) return; // 不能给自己发私聊
    
    // 打开或激活私聊窗口
    if (!privateChatWindows.contains(selectedUser)) {
        PrivateChatWindow *privateChatWindow = new PrivateChatWindow(selectedUser, currentNickname, client, this);
        privateChatWindows[selectedUser] = privateChatWindow;
        
        // 当私聊窗口关闭时从映射中移除
        connect(privateChatWindow, &QDialog::finished, [this, selectedUser]() {
            privateChatWindows.remove(selectedUser);
        });
    }
    
    // 显示并激活窗口
    privateChatWindows[selectedUser]->show();
    privateChatWindows[selectedUser]->raise();
    privateChatWindows[selectedUser]->activateWindow();
}

void MainWindow::changeNickname()
{
    if (!client || !client->isConnected()) {
        QMessageBox::warning(this, "警告", "未连接到服务器");
        return;
    }
    
    bool ok;
    QString newNickname = QInputDialog::getText(this, "更改昵称", "新昵称:", 
                                               QLineEdit::Normal, currentNickname, &ok);
    if (ok && !newNickname.isEmpty() && newNickname != currentNickname) {
        client->changeNickname(newNickname);
        currentNickname = newNickname;
        statusLabel->setText(QString("已连接 - %1").arg(currentNickname));
    }
}

void MainWindow::onClientConnected(const QString &clientInfo)
{
    if (server) {
        addMessageToChat("系统", QString("客户端连接: %1").arg(clientInfo), QDateTime::currentDateTime(), false, true);
        // 更新客户端列表
        QStringList clients = server->getConnectedClients();
        updateUserList(clients);
    }
}

void MainWindow::onClientDisconnected(const QString &clientInfo)
{
    if (server) {
        addMessageToChat("系统", QString("客户端断开: %1").arg(clientInfo), QDateTime::currentDateTime(), false, true);
        // 更新客户端列表
        QStringList clients = server->getConnectedClients();
        updateUserList(clients);
    }
}

void MainWindow::onServerMessageReceived(const QString &sender, const QString &message)
{
    addMessageToChat(sender, message, QDateTime::currentDateTime());
    saveMessageToDatabase(sender, message);
}

void MainWindow::sendFile()
{
    if (!client || !client->isConnected()) {
        QMessageBox::warning(this, "警告", "未连接到服务器");
        return;
    }
    
    QStringList users = client->getOnlineUsers();
    if (users.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有在线用户");
        return;
    }
    
    // 选择接收用户
    bool ok;
    QString receiver = QInputDialog::getItem(this, "选择接收者", "选择文件接收者:", 
                                           users, 0, false, &ok);
    if (!ok || receiver.isEmpty() || receiver == currentNickname) {
        return;
    }
    
    // 选择文件
    QString fileName = QFileDialog::getOpenFileName(this, "选择要发送的文件", 
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                   "所有文件 (*)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(fileName);
    qint64 fileSize = fileInfo.size();
    
    // 调试信息
    qDebug() << "选择的文件:" << fileName;
    qDebug() << "文件大小:" << fileSize << "字节";
    qDebug() << "文件是否存在:" << fileInfo.exists();
    qDebug() << "文件是否可读:" << fileInfo.isReadable();
    
    if (fileSize == 0) {
        QMessageBox::warning(this, "警告", "选择的文件为空或无法读取");
        return;
    }
    
    if (fileSize > 100 * 1024 * 1024) { // 限制100MB
        QMessageBox::warning(this, "警告", "文件大小超过100MB限制");
        return;
    }
    
    // 创建文件传输对象
    if (fileTransfer) {
        fileTransfer->deleteLater();
    }
    fileTransfer = new FileTransfer(this);
    connect(fileTransfer, &FileTransfer::transferStarted, this, &MainWindow::onTransferStarted);
    connect(fileTransfer, &FileTransfer::transferProgress, this, &MainWindow::onTransferProgress);
    connect(fileTransfer, &FileTransfer::transferCompleted, this, &MainWindow::onTransferCompleted);
    connect(fileTransfer, &FileTransfer::transferFailed, this, &MainWindow::onTransferFailed);
    
    // 发送文件传输请求，等待对方响应
    pendingFileTransferSender = receiver;
    pendingFileName = fileName;
    pendingFileSize = fileSize;
    
    qDebug() << "请求文件传输:" << fileInfo.fileName() << "大小:" << fileSize;
    client->requestFileTransfer(receiver, fileInfo.fileName(), fileSize, 0); // 端口为0，等待响应
}

void MainWindow::onFileTransferRequested(const QString &sender, const QString &fileName, qint64 fileSize)
{
    showFileTransferDialog(sender, fileName, fileSize);
}

void MainWindow::onFileTransferResponse(const QString &responder, bool accepted, quint16 transferPort)
{
    if (accepted) {
        addMessageToChat("系统", QString("%1 接受了文件传输，开始传输...").arg(responder), 
                        QDateTime::currentDateTime(), false, true);
        
        // 延迟一下让接收方准备好
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [this, timer, transferPort]() {
            if (fileTransfer && !pendingFileName.isEmpty()) {
                if (!fileTransfer->sendFile(pendingFileName, "127.0.0.1", transferPort)) {
                    QMessageBox::critical(this, "错误", "无法启动文件传输");
                    onTransferFailed("无法连接到接收方");
                }
            }
            timer->deleteLater();
        });
        timer->start(1000);
    } else {
        addMessageToChat("系统", QString("%1 拒绝了文件传输").arg(responder), 
                        QDateTime::currentDateTime(), false, true);
        if (fileTransfer) {
            fileTransfer->cancelTransfer();
        }
        // 清理pending状态
        pendingFileTransferSender.clear();
        pendingFileName.clear();
        pendingFileSize = 0;
    }
}

void MainWindow::onTransferStarted()
{
    transferProgressBar->setVisible(true);
    transferProgressBar->setValue(0);
    addMessageToChat("系统", "文件传输开始", QDateTime::currentDateTime(), false, true);
}

void MainWindow::onTransferProgress(int percentage, qint64 bytesTransferred, qint64 totalBytes)
{
    transferProgressBar->setValue(percentage);
    transferProgressBar->setFormat(QString("传输进度: %1% (%2 / %3)")
                                  .arg(percentage)
                                  .arg(formatFileSize(bytesTransferred))
                                  .arg(formatFileSize(totalBytes)));
}

void MainWindow::onTransferCompleted()
{
    transferProgressBar->setVisible(false);
    addMessageToChat("系统", "文件传输完成", QDateTime::currentDateTime(), false, true);
    
    if (fileTransfer) {
        fileTransfer->deleteLater();
        fileTransfer = nullptr;
    }
    
    // 清理pending状态
    pendingFileTransferSender.clear();
    pendingFileName.clear();
    pendingFileSize = 0;
}

void MainWindow::onTransferFailed(const QString &error)
{
    transferProgressBar->setVisible(false);
    addMessageToChat("系统", QString("文件传输失败: %1").arg(error), 
                    QDateTime::currentDateTime(), false, true);
    QMessageBox::critical(this, "文件传输失败", error);
    
    if (fileTransfer) {
        fileTransfer->deleteLater();
        fileTransfer = nullptr;
    }
    
    // 清理pending状态
    pendingFileTransferSender.clear();
    pendingFileName.clear();
    pendingFileSize = 0;
}

void MainWindow::changeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, currentMessageFormat.font(), this);
    if (ok) {
        currentMessageFormat.setFont(font);
        messageInput->setCurrentCharFormat(currentMessageFormat);
    }
}

void MainWindow::changeTextColor()
{
    QColor color = QColorDialog::getColor(currentMessageFormat.foreground().color(), this);
    if (color.isValid()) {
        currentMessageFormat.setForeground(color);
        messageInput->setCurrentCharFormat(currentMessageFormat);
    }
}

void MainWindow::showChatHistory()
{
    if (!database) {
        QMessageBox::warning(this, "警告", "数据库不可用");
        return;
    }
    
    // 创建聊天记录对话框
    QDialog* historyDialog = new QDialog(this);
    historyDialog->setWindowTitle("聊天记录");
    historyDialog->resize(600, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(historyDialog);
    
    // 查询选项
    QHBoxLayout* queryLayout = new QHBoxLayout();
    QPushButton* allBtn = new QPushButton("所有记录");
    QPushButton* dateBtn = new QPushButton("按日期查询");
    QPushButton* senderBtn = new QPushButton("按发送者查询");
    
    queryLayout->addWidget(allBtn);
    queryLayout->addWidget(dateBtn);
    queryLayout->addWidget(senderBtn);
    queryLayout->addStretch();
    
    layout->addLayout(queryLayout);
    
    // 显示区域
    QTextEdit* historyDisplay = new QTextEdit();
    historyDisplay->setReadOnly(true);
    layout->addWidget(historyDisplay);
    
    // 显示所有记录
    auto showAllHistory = [=]() {
        QList<ChatMessage> messages = database->getMessages(200);
        historyDisplay->clear();
        for (const auto& msg : messages) {
            QString displayText = QString("[%1] %2: %3")
                                .arg(formatTimestamp(msg.timestamp))
                                .arg(msg.sender)
                                .arg(msg.content);
            historyDisplay->append(displayText);
        }
    };
    
    connect(allBtn, &QPushButton::clicked, showAllHistory);
    
    // 按日期查询
    connect(dateBtn, &QPushButton::clicked, [=]() {
        bool ok;
        QString dateStr = QInputDialog::getText(historyDialog, "选择日期", "请输入日期(yyyy-MM-dd):", 
                                               QLineEdit::Normal, QDate::currentDate().toString("yyyy-MM-dd"), &ok);
        if (ok && !dateStr.isEmpty()) {
            QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
            if (date.isValid()) {
                QList<ChatMessage> messages = database->getMessagesByDate(date);
                historyDisplay->clear();
                for (const auto& msg : messages) {
                    QString displayText = QString("[%1] %2: %3")
                                        .arg(formatTimestamp(msg.timestamp))
                                        .arg(msg.sender)
                                        .arg(msg.content);
                    historyDisplay->append(displayText);
                }
            } else {
                QMessageBox::warning(historyDialog, "错误", "日期格式不正确，请使用 yyyy-MM-dd 格式");
            }
        }
    });
    
    // 按发送者查询
    connect(senderBtn, &QPushButton::clicked, [=]() {
        bool ok;
        QString sender = QInputDialog::getText(historyDialog, "查询发送者", "发送者昵称:", 
                                             QLineEdit::Normal, "", &ok);
        if (ok && !sender.isEmpty()) {
            QList<ChatMessage> messages = database->getMessagesBySender(sender);
            historyDisplay->clear();
            for (const auto& msg : messages) {
                QString displayText = QString("[%1] %2: %3")
                                    .arg(formatTimestamp(msg.timestamp))
                                    .arg(msg.sender)
                                    .arg(msg.content);
                historyDisplay->append(displayText);
            }
        }
    });
    
    // 默认显示所有记录
    showAllHistory();
    
    historyDialog->exec();
    historyDialog->deleteLater();
}

void MainWindow::clearChatHistory()
{
    int ret = QMessageBox::question(this, "确认", "确定要清空所有聊天记录吗？", 
                                  QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes && database) {
        if (database->clearMessages()) {
            QMessageBox::information(this, "成功", "聊天记录已清空");
            chatDisplay->clear();
        } else {
            QMessageBox::critical(this, "错误", "清空聊天记录失败");
        }
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于", 
                      "现代聊天室 v1.0\n\n"
                      "功能特性：\n"
                      "• TCP/UDP 网络通信\n"
                      "• 群聊和私聊支持\n"
                      "• 点对点文件传输\n"
                      "• SQLite 聊天记录存储\n"
                      "• 现代扁平化 UI 设计\n"
                      "• 字体和颜色自定义\n\n"
                      "使用方法：\n"
                      "1. 选择服务器模式或客户端模式\n"
                      "2. 输入连接信息\n"
                      "3. 开始聊天和文件传输\n\n"
                      "快捷键：\n"
                      "• Ctrl+Enter: 发送消息\n"
                      "• 双击用户名: 发送私聊");
}

// 辅助函数实现
void MainWindow::addMessageToChat(const QString &sender, const QString &message, 
                                 const QDateTime &timestamp, bool isPrivate, bool isSystem)
{
    QString timeStr = formatTimestamp(timestamp);
    QString displayText;
    
    if (isSystem) {
        displayText = QString("<font color='#666666'>[%1] <b>系统:</b> %2</font>")
                     .arg(timeStr).arg(message.toHtmlEscaped());
    } else if (isPrivate) {
        displayText = QString("<font color='#ff6600'>[%1] <b>%2:</b> %3</font>")
                     .arg(timeStr).arg(sender.toHtmlEscaped()).arg(message.toHtmlEscaped());
    } else {
        displayText = QString("[%1] <b>%2:</b> %3")
                     .arg(timeStr).arg(sender.toHtmlEscaped()).arg(message.toHtmlEscaped());
    }
    
    chatDisplay->append(displayText);
    
    // 自动滚动到底部
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
}

void MainWindow::addFormattedMessageToChat(const QString &sender, const QString &formattedMessage, 
                                         const QDateTime &timestamp, bool isPrivate, bool isSystem)
{
    QString timeStr = formatTimestamp(timestamp);
    QString displayText;
    
    if (isSystem) {
        displayText = QString("<font color='#666666'>[%1] <b>系统:</b> %2</font>")
                     .arg(timeStr).arg(formattedMessage);
    } else if (isPrivate) {
        displayText = QString("<font color='#ff6600'>[%1] <b>%2:</b> %3</font>")
                     .arg(timeStr).arg(sender.toHtmlEscaped()).arg(formattedMessage);
    } else {
        displayText = QString("[%1] <b>%2:</b> %3")
                     .arg(timeStr).arg(sender.toHtmlEscaped()).arg(formattedMessage);
    }
    
    chatDisplay->append(displayText);
    
    // 自动滚动到底部
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
}

void MainWindow::updateUserList(const QStringList &users)
{
    userListWidget->clear();
    for (const QString& user : users) {
        userListWidget->addItem(user);
    }
}

void MainWindow::showFileTransferDialog(const QString &sender, const QString &fileName, qint64 fileSize)
{
    int ret = QMessageBox::question(this, "文件传输请求", 
                                  QString("%1 想要发送文件给您:\n\n"
                                         "文件名: %2\n"
                                         "大小: %3\n\n"
                                         "是否接受？")
                                  .arg(sender)
                                  .arg(fileName)
                                  .arg(formatFileSize(fileSize)),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // 选择保存位置
        QString saveDir = QFileDialog::getExistingDirectory(this, "选择保存位置", 
                                                          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if (!saveDir.isEmpty()) {
            // 创建文件接收器
            if (currentFileReceiver) {
                currentFileReceiver->deleteLater();
            }
            currentFileReceiver = new FileTransfer(this);
            connect(currentFileReceiver, &FileTransfer::transferStarted, this, &MainWindow::onTransferStarted);
            connect(currentFileReceiver, &FileTransfer::transferProgress, this, &MainWindow::onTransferProgress);
            connect(currentFileReceiver, &FileTransfer::transferCompleted, this, &MainWindow::onTransferCompleted);
            connect(currentFileReceiver, &FileTransfer::transferFailed, this, &MainWindow::onTransferFailed);
            
            // 动态分配端口，避免冲突
            quint16 transferPort = 10000 + QRandomGenerator::global()->bounded(1000); // 10000-10999范围
            if (currentFileReceiver->receiveFile(saveDir, transferPort)) {
                client->respondToFileTransfer(sender, true, transferPort);
                addMessageToChat("系统", QString("正在等待文件传输连接...端口: %1").arg(transferPort), 
                                QDateTime::currentDateTime(), false, true);
            } else {
                QMessageBox::critical(this, "错误", "无法启动文件接收");
                client->respondToFileTransfer(sender, false, 0);
            }
        } else {
            client->respondToFileTransfer(sender, false, 0);
        }
    } else {
        client->respondToFileTransfer(sender, false, 0);
    }
}

QString MainWindow::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " Bytes";
    }
}

QString MainWindow::formatTimestamp(const QDateTime &timestamp)
{
    return timestamp.toString("hh:mm:ss");
}

void MainWindow::loadChatHistory()
{
    if (!database) return;
    
    QList<ChatMessage> messages = database->getMessages(50); // 加载最近50条消息
    for (const auto& msg : messages) {
        addMessageToChat(msg.sender, msg.content, msg.timestamp, false, msg.sender == "系统");
    }
}

void MainWindow::saveMessageToDatabase(const QString &sender, const QString &message, 
                                      bool isFile, const QString &filePath)
{
    if (database) {
        database->addMessage(sender, message, isFile, filePath);
    }
}

void MainWindow::updateUserListWithPrivateMessageIndicator(const QString &sender)
{
    // 在用户列表中为有私聊消息的用户添加指示器
    for (int i = 0; i < userListWidget->count(); ++i) {
        QListWidgetItem *item = userListWidget->item(i);
        if (item && item->text().contains(sender)) {
            // 如果已经有指示器，不重复添加
            if (!item->text().startsWith("🔴")) {
                item->setText(QString("🔴 %1").arg(sender));
                item->setToolTip(QString("有来自 %1 的新私聊消息").arg(sender));
            }
            break;
        }
    }
}
