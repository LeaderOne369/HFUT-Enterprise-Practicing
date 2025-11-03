#include "privatechatwindow.h"
#include "chatclient.h"
#include <QKeyEvent>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QTextCursor>
#include <QIcon>
#include <QRandomGenerator>
#include <QGridLayout>
#include <QScrollArea>
#include <QFileDialog>
#include <QFileInfo>

PrivateChatWindow::PrivateChatWindow(const QString &targetUser, const QString &currentUser, 
                                   ChatClient *client, QWidget *parent)
    : QDialog(parent), targetUser(targetUser), currentUser(currentUser), chatClient(client)
{
    setupUI();
    applyIOSStyle();
    loadChatHistory();
    setWindowTitle(QString("私聊 - %1").arg(targetUser));
    resize(480, 600);
    
    // 安装事件过滤器以处理 Ctrl+Enter
    messageInput->installEventFilter(this);
    
    // 设置窗口图标
    setWindowIcon(QIcon(":/icons/chat.png"));
    
    // 窗口属性
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);
}

PrivateChatWindow::~PrivateChatWindow()
{
}

void PrivateChatWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 顶部标题区域
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    
    // 用户头像（使用文字头像）
    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setText(targetUser.left(1).toUpper());
    avatarLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #007aff;"
        "   color: white;"
        "   border-radius: 20px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
    );
    
    // 标题和状态
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLabel = new QLabel(QString("与 %1 的私聊").arg(targetUser));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1c1c1e;");
    
    statusLabel = new QLabel("在线");
    statusLabel->setStyleSheet("font-size: 14px; color: #8e8e93;");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(statusLabel);
    titleLayout->setSpacing(2);
    
    headerLayout->addWidget(avatarLabel);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    
    mainLayout->addWidget(headerWidget);
    
    // 聊天显示区域
    chatDisplay = new QTextEdit();
    chatDisplay->setReadOnly(true);
    chatDisplay->setObjectName("privateChatDisplay");
    mainLayout->addWidget(chatDisplay);
    
    // 输入区域容器
    QWidget *inputContainer = new QWidget();
    QVBoxLayout *inputContainerLayout = new QVBoxLayout(inputContainer);
    inputContainerLayout->setContentsMargins(0, 0, 0, 0);
    inputContainerLayout->setSpacing(12);
    
    // 工具栏
    QWidget *toolBar = new QWidget();
    QHBoxLayout *toolLayout = new QHBoxLayout(toolBar);
    toolLayout->setContentsMargins(0, 0, 0, 0);
    
    QPushButton *emojiButton = new QPushButton("😊");
    emojiButton->setFixedSize(36, 36);
    emojiButton->setObjectName("emojiButton");
    
    QPushButton *fileButton = new QPushButton("📎");
    fileButton->setFixedSize(36, 36);
    fileButton->setObjectName("fileButton");
    
    toolLayout->addWidget(emojiButton);
    toolLayout->addWidget(fileButton);
    toolLayout->addStretch();
    
    // 消息输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(12);
    
    messageInput = new QTextEdit();
    messageInput->setMaximumHeight(100);
    messageInput->setMinimumHeight(44);
    messageInput->setPlaceholderText("输入私聊消息...");
    messageInput->setObjectName("privateMessageInput");
    
    sendButton = new QPushButton("发送");
    sendButton->setFixedSize(80, 44);
    sendButton->setObjectName("privateSendButton");
    
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);
    
    inputContainerLayout->addWidget(toolBar);
    inputContainerLayout->addLayout(inputLayout);
    
    mainLayout->addWidget(inputContainer);
    
    // 连接信号
    connect(sendButton, &QPushButton::clicked, this, &PrivateChatWindow::sendMessage);
    connect(emojiButton, &QPushButton::clicked, this, &PrivateChatWindow::insertEmoji);
    connect(fileButton, &QPushButton::clicked, this, &PrivateChatWindow::sendFile);
}

void PrivateChatWindow::addMessage(const QString &sender, const QString &message, const QDateTime &timestamp)
{
    QString timeStr = timestamp.toString("hh:mm:ss");
    QString displayText;
    
    // 检查是否是当前用户发送的消息
    bool isMyMessage = (sender.trimmed() == currentUser.trimmed());
    
    if (isMyMessage) {
        // 本人发送的消息
        displayText = QString("[%1] 我: %2")
                      .arg(timeStr)
                      .arg(message.toHtmlEscaped());
    } else {
        // 对方发送的消息
        displayText = QString("[%1] %2: %3")
                      .arg(timeStr)
                      .arg(sender)
                      .arg(message.toHtmlEscaped());
        
        // 显示新消息通知
        showNotification(message);
    }
    
    chatDisplay->append(displayText);
    
    // 自动滚动到底部
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
    
    // 保存消息到历史记录
    saveMessage(sender, message, timestamp);
}

void PrivateChatWindow::sendMessage()
{
    QString message = messageInput->toPlainText().trimmed();
    if (message.isEmpty() || !chatClient) return;
    
    // 发送私聊消息
    chatClient->sendPrivateMessage(targetUser, message);
    
    // 在本地显示消息
    QDateTime now = QDateTime::currentDateTime();
    addMessage(currentUser, message, now);
    
    // 清空输入框
    messageInput->clear();
    messageInput->setFocus();
}

bool PrivateChatWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return && keyEvent->modifiers() == Qt::ControlModifier) {
            sendMessage();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void PrivateChatWindow::applyIOSStyle()
{
    // iOS风格的私聊窗口样式
    QString iosPrivateStyle = R"(
        PrivateChatWindow {
            background-color: #f2f2f7;
        }
        
        QTextEdit[objectName="privateChatDisplay"] {
            background-color: #ffffff;
            border: 2px solid #e5e5ea;
            border-radius: 16px;
            padding: 16px;
            font-size: 16px;
            color: #1c1c1e;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
            selection-background-color: #007aff;
            selection-color: white;
        }
        
        QTextEdit[objectName="privateMessageInput"] {
            background-color: #f2f2f7;
            border: 2px solid #e5e5ea;
            border-radius: 22px;
            padding: 12px 16px;
            font-size: 16px;
            color: #1c1c1e;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
        }
        
        QTextEdit[objectName="privateMessageInput"]:focus {
            border-color: #007aff;
            background-color: #ffffff;
        }
        
        QPushButton[objectName="privateSendButton"] {
            background-color: #007aff;
            color: white;
            border: none;
            border-radius: 22px;
            font-size: 16px;
            font-weight: 600;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
        }
        
        QPushButton[objectName="privateSendButton"]:hover {
            background-color: #0056b3;
        }
        
        QPushButton[objectName="privateSendButton"]:pressed {
            background-color: #004494;
        }
        
        QPushButton[objectName="emojiButton"],
        QPushButton[objectName="fileButton"] {
            background-color: #f2f2f7;
            border: 1px solid #e5e5ea;
            border-radius: 18px;
            font-size: 16px;
        }
        
        QPushButton[objectName="emojiButton"]:hover,
        QPushButton[objectName="fileButton"]:hover {
            background-color: #e5e5ea;
        }
        
        QScrollBar:vertical {
            background-color: #f2f2f7;
            width: 6px;
            border-radius: 3px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #c6c6c8;
            border-radius: 3px;
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
    )";
    
    setStyleSheet(iosPrivateStyle);
}

void PrivateChatWindow::loadChatHistory()
{
    QString chatDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/private_chats";
    QDir dir(chatDir);
    if (!dir.exists()) {
        dir.mkpath(chatDir);
        return;
    }
    
    QString fileName = QString("%1_%2.json").arg(currentUser).arg(targetUser);
    QString filePath = chatDir + "/" + fileName;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray messages = doc.array();
    
    chatDisplay->clear();
    for (const QJsonValue &value : messages) {
        QJsonObject msg = value.toObject();
        QString sender = msg["sender"].toString();
        QString message = msg["message"].toString();
        QString timestampStr = msg["timestamp"].toString();
        QDateTime timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
        
        addMessage(sender, message, timestamp);
    }
}

void PrivateChatWindow::saveChatHistory()
{
    QString chatDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/private_chats";
    QDir dir(chatDir);
    if (!dir.exists()) {
        dir.mkpath(chatDir);
    }
    
    QString fileName = QString("%1_%2.json").arg(currentUser).arg(targetUser);
    QString filePath = chatDir + "/" + fileName;
    
    // 读取现有消息
    QJsonArray messages;
    QFile existingFile(filePath);
    if (existingFile.open(QIODevice::ReadOnly)) {
        QByteArray data = existingFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        messages = doc.array();
        existingFile.close();
    }
    
    // 只保存最近的100条消息
    while (messages.size() > 100) {
        messages.removeFirst();
    }
    
    // 保存到文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(messages);
        file.write(doc.toJson());
    }
}

void PrivateChatWindow::saveMessage(const QString &sender, const QString &message, const QDateTime &timestamp)
{
    QString chatDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/private_chats";
    QDir dir(chatDir);
    if (!dir.exists()) {
        dir.mkpath(chatDir);
    }
    
    QString fileName = QString("%1_%2.json").arg(currentUser).arg(targetUser);
    QString filePath = chatDir + "/" + fileName;
    
    // 读取现有消息
    QJsonArray messages;
    QFile existingFile(filePath);
    if (existingFile.open(QIODevice::ReadOnly)) {
        QByteArray data = existingFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        messages = doc.array();
        existingFile.close();
    }
    
    // 添加新消息
    QJsonObject newMessage;
    newMessage["sender"] = sender;
    newMessage["message"] = message;
    newMessage["timestamp"] = timestamp.toString(Qt::ISODate);
    messages.append(newMessage);
    
    // 只保存最近的100条消息
    while (messages.size() > 100) {
        messages.removeFirst();
    }
    
    // 保存到文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(messages);
        file.write(doc.toJson());
    }
}

void PrivateChatWindow::insertEmoji()
{
    // 创建emoji选择器对话框
    QDialog *emojiDialog = new QDialog(this);
    emojiDialog->setWindowTitle("选择 Emoji");
    emojiDialog->setFixedSize(400, 300);
    emojiDialog->setStyleSheet(
        "QDialog {"
        "   background-color: #f2f2f7;"
        "   border-radius: 16px;"
        "}"
        "QPushButton {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e5e5ea;"
        "   border-radius: 8px;"
        "   font-size: 24px;"
        "   padding: 8px;"
        "   margin: 2px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #007aff;"
        "   border-color: #007aff;"
        "}"
    );
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(emojiDialog);
    
    // 标题
    QLabel *titleLabel = new QLabel("选择一个 Emoji 表情");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1c1c1e; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    dialogLayout->addWidget(titleLabel);
    
    // emoji网格
    QGridLayout *emojiGrid = new QGridLayout();
    
    // 常用emoji列表
    QStringList emojis = {
        "😀", "😃", "😄", "😁", "😅", "😂", "🤣", "😊", "😇", "🙂",
        "🙃", "😉", "😌", "😍", "🥰", "😘", "😗", "😙", "😚", "😋",
        "😛", "😝", "😜", "🤪", "🤨", "🧐", "🤓", "😎", "🤩", "🥳",
        "😏", "😒", "😞", "😔", "😟", "😕", "🙁", "☹️", "😣", "😖",
        "😫", "😩", "🥺", "😢", "😭", "😤", "😠", "😡", "🤬", "🤯",
        "😳", "🥵", "🥶", "😱", "😨", "😰", "😥", "😓", "🤗", "🤔",
        "👍", "👎", "👌", "✌️", "🤞", "🤟", "🤘", "🤙", "👈", "👉",
        "👆", "🖕", "👇", "☝️", "👋", "🤚", "🖐️", "✋", "🖖", "👏",
        "🙌", "🤲", "🙏", "✍️", "💅", "🤳", "💪", "🦾", "🦿", "🦵",
        "🦶", "👂", "🦻", "👃", "🧠", "🦷", "🦴", "👀", "👁️", "👅"
    };
    
    int row = 0, col = 0;
    for (const QString &emoji : emojis) {
        QPushButton *emojiBtn = new QPushButton(emoji);
        emojiBtn->setFixedSize(40, 40);
        
        connect(emojiBtn, &QPushButton::clicked, [this, emoji, emojiDialog]() {
            messageInput->insertPlainText(emoji);
            messageInput->setFocus();
            emojiDialog->accept();
        });
        
        emojiGrid->addWidget(emojiBtn, row, col);
        col++;
        if (col >= 10) {
            col = 0;
            row++;
        }
    }
    
    QWidget *emojiWidget = new QWidget();
    emojiWidget->setLayout(emojiGrid);
    
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(emojiWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e5e5ea;"
        "   border-radius: 8px;"
        "}"
    );
    
    dialogLayout->addWidget(scrollArea);
    
    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #8e8e93;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 12px 24px;"
        "   font-size: 16px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "   background-color: #6d6d70;"
        "}"
    );
    connect(closeBtn, &QPushButton::clicked, emojiDialog, &QDialog::reject);
    dialogLayout->addWidget(closeBtn);
    
    emojiDialog->exec();
    delete emojiDialog;
}

void PrivateChatWindow::sendFile()
{
    if (!chatClient) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }
    
    // 选择文件
    QString fileName = QFileDialog::getOpenFileName(this, "选择要发送的文件", 
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                   "所有文件 (*);;图片文件 (*.png *.jpg *.jpeg *.gif);;文档文件 (*.pdf *.doc *.docx *.txt)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(fileName);
    qint64 fileSize = fileInfo.size();
    
    if (fileSize == 0) {
        QMessageBox::warning(this, "错误", "选择的文件为空或无法读取");
        return;
    }
    
    if (fileSize > 100 * 1024 * 1024) { // 限制100MB
        QMessageBox::warning(this, "错误", "文件大小超过100MB限制");
        return;
    }
    
    // 发送文件传输请求
    // 注意：这里需要扩展ChatClient类来支持私聊文件传输
    // 暂时显示文件信息和成功提示
    QString fileSizeStr = formatFileSize(fileSize);
    
    QMessageBox::information(this, "文件选择成功", 
                           QString("文件: %1\n大小: %2\n\n注意：当前版本暂不支持私聊文件传输，"
                                  "请使用主窗口的文件传输功能。")
                           .arg(fileInfo.fileName())
                           .arg(fileSizeStr));
                           
    // TODO: 实现真正的私聊文件传输
    // chatClient->sendPrivateFile(targetUser, fileName);
}

QString PrivateChatWindow::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString::number(bytes / GB, 'f', 1) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / MB, 'f', 1) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / KB, 'f', 1) + " KB";
    } else {
        return QString::number(bytes) + " 字节";
    }
}

void PrivateChatWindow::showNotification(const QString &message)
{
    // 系统通知
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QSystemTrayIcon trayIcon;
        trayIcon.setIcon(QIcon(":/icons/chat.png"));
        trayIcon.show();
        trayIcon.showMessage(
            QString("来自 %1 的私聊消息").arg(targetUser),
            message,
            QSystemTrayIcon::Information,
            3000
        );
    }
    
    // 窗口闪烁提醒
    if (!isActiveWindow()) {
        QApplication::alert(this, 3000);
        
        // 修改窗口标题以提醒有新消息
        QString originalTitle = windowTitle();
        setWindowTitle(QString("🔴 %1").arg(originalTitle));
        
        // 3秒后恢复标题
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [this, timer, originalTitle]() {
            setWindowTitle(originalTitle);
            timer->deleteLater();
        });
        timer->start(3000);
    }
}

void PrivateChatWindow::closeEvent(QCloseEvent *event)
{
    saveChatHistory();
    QDialog::closeEvent(event);
} 