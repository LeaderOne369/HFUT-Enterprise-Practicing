#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , udpSocket(nullptr)
    , sendTimer(new QTimer(this))
    , isPortBound(false)
{
    ui->setupUi(this);
    setupUI();
    
    // 获取本地IP
    localIP = getLocalIP();
    localIpValue->setText(localIP);
    
    // 设置定时器
    connect(sendTimer, &QTimer::timeout, this, &MainWindow::onTimerSend);
    
    // 设置窗口属性
    setWindowTitle("网络调试助手");
    resize(900, 700);
    
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

void MainWindow::setupUI()
{
    // 创建中央组件
    QWidget *centralWidget = new QWidget;
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
    } else {
        receiveTextEdit->append(QString("[%1] 发送到 %2:%3 - %4")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(targetIp)
                              .arg(targetPort)
                              .arg(data));
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
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        
        QHostAddress sender;
        quint16 senderPort;
        
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        
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
