#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QHostAddress>
#include <QInputDialog>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentSelectedCard(-1)
{
    ui->setupUi(this);
    setupUI();
    setupStyles();
    loadNetworkCards();
    populateNetworkTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("网卡管理工具");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // 创建中央部件
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧面板 - 网卡列表
    networkListGroup = new QGroupBox("网卡列表", this);
    networkListGroup->setMinimumWidth(600);
    
    QVBoxLayout *leftLayout = new QVBoxLayout(networkListGroup);
    
    // 刷新按钮
    refreshButton = new QPushButton("🔄 刷新网卡列表", this);
    refreshButton->setMaximumHeight(40);
    leftLayout->addWidget(refreshButton);
    
    // 网卡表格
    networkTable = new QTableWidget(this);
    networkTable->setColumnCount(6);
    QStringList headers = {"网卡名称", "显示名称", "MAC地址", "IP地址", "状态", "类型"};
    networkTable->setHorizontalHeaderLabels(headers);
    networkTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    networkTable->setAlternatingRowColors(true);
    networkTable->horizontalHeader()->setStretchLastSection(true);
    networkTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftLayout->addWidget(networkTable);
    
    // 右侧面板 - 配置区域
    configGroup = new QGroupBox("网卡配置", this);
    configGroup->setMinimumWidth(500);
    
    QVBoxLayout *rightLayout = new QVBoxLayout(configGroup);
    
    // 选中网卡信息
    selectedCardLabel = new QLabel("<h3>请选择一个网卡</h3>", this);
    selectedCardLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(selectedCardLabel);
    
    // 基本信息框架
    QFrame *infoFrame = new QFrame(this);
    infoFrame->setFrameStyle(QFrame::StyledPanel);
    QGridLayout *infoLayout = new QGridLayout(infoFrame);
    
    infoLayout->addWidget(new QLabel("网卡名称:"), 0, 0);
    cardNameEdit = new QLineEdit(this);
    cardNameEdit->setReadOnly(true);
    infoLayout->addWidget(cardNameEdit, 0, 1);
    
    infoLayout->addWidget(new QLabel("MAC地址:"), 1, 0);
    macAddressLabel = new QLabel("-", this);
    infoLayout->addWidget(macAddressLabel, 1, 1);
    
    infoLayout->addWidget(new QLabel("状态:"), 2, 0);
    statusLabel = new QLabel("-", this);
    infoLayout->addWidget(statusLabel, 2, 1);
    
    infoLayout->addWidget(new QLabel("类型:"), 3, 0);
    typeLabel = new QLabel("-", this);
    infoLayout->addWidget(typeLabel, 3, 1);
    
    infoLayout->addWidget(new QLabel("MTU:"), 4, 0);
    mtuLabel = new QLabel("-", this);
    infoLayout->addWidget(mtuLabel, 4, 1);
    
    rightLayout->addWidget(infoFrame);
    
    // IP配置区域
    ipConfigGroup = new QGroupBox("IP地址配置", this);
    QVBoxLayout *ipLayout = new QVBoxLayout(ipConfigGroup);
    
    // IP地址表格
    ipTable = new QTableWidget(this);
    ipTable->setColumnCount(3);
    QStringList ipHeaders = {"IP地址", "子网掩码", "类型"};
    ipTable->setHorizontalHeaderLabels(ipHeaders);
    ipTable->setAlternatingRowColors(true);
    ipTable->horizontalHeader()->setStretchLastSection(true);
    ipTable->setMaximumHeight(200);
    ipLayout->addWidget(ipTable);
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addIpButton = new QPushButton("➕ 添加IP", this);
    removeIpButton = new QPushButton("➖ 删除IP", this);
    modifyIpButton = new QPushButton("✏️ 修改IP", this);
    applyButton = new QPushButton("✅ 应用更改", this);
    
    addIpButton->setEnabled(false);
    removeIpButton->setEnabled(false);
    modifyIpButton->setEnabled(false);
    applyButton->setEnabled(false);
    
    buttonLayout->addWidget(addIpButton);
    buttonLayout->addWidget(removeIpButton);
    buttonLayout->addWidget(modifyIpButton);
    buttonLayout->addWidget(applyButton);
    
    ipLayout->addLayout(buttonLayout);
    rightLayout->addWidget(ipConfigGroup);
    
    // 添加到分割器
    mainSplitter->addWidget(networkListGroup);
    mainSplitter->addWidget(configGroup);
    mainSplitter->setSizes({600, 500});
    
    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(mainSplitter);
    
    // 连接信号和槽
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshNetworkCards);
    connect(networkTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onNetworkCardSelectionChanged);
    connect(addIpButton, &QPushButton::clicked, this, &MainWindow::addIPAddress);
    connect(removeIpButton, &QPushButton::clicked, this, &MainWindow::removeIPAddress);
    connect(modifyIpButton, &QPushButton::clicked, this, &MainWindow::modifyIPAddress);
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::applyNetworkChanges);
}

void MainWindow::setupStyles()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f0f0f0;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
            background-color: white;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #2c3e50;
            font-size: 14px;
        }
        
        QTableWidget {
            gridline-color: #e0e0e0;
            background-color: white;
            alternate-background-color: #f8f9fa;
            selection-background-color: #3498db;
            selection-color: white;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        
        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #e0e0e0;
        }
        
        QHeaderView::section {
            background-color: #34495e;
            color: white;
            padding: 8px;
            border: none;
            font-weight: bold;
        }
        
        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
            min-height: 20px;
        }
        
        QPushButton:hover {
            background-color: #2980b9;
        }
        
        QPushButton:pressed {
            background-color: #21618c;
        }
        
        QPushButton:disabled {
            background-color: #bdc3c7;
            color: #7f8c8d;
        }
        
        QLineEdit {
            padding: 8px;
            border: 2px solid #ddd;
            border-radius: 4px;
            background-color: white;
        }
        
        QLineEdit:focus {
            border-color: #3498db;
        }
        
        QLabel {
            color: #2c3e50;
        }
        
        QFrame {
            background-color: #f8f9fa;
            border-radius: 4px;
        }
    )");
}

void MainWindow::loadNetworkCards()
{
    networkCards.clear();
    
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface &interface : interfaces) {
        NetworkCardInfo info;
        info.name = interface.name();
        info.displayName = interface.humanReadableName();
        info.macAddress = interface.hardwareAddress();
        info.mtu = interface.maximumTransmissionUnit();
        
        // 获取状态
        if (interface.flags() & QNetworkInterface::IsUp) {
            info.status = "活动";
        } else {
            info.status = "未活动";
        }
        
        // 获取类型
        if (interface.type() == QNetworkInterface::Ethernet) {
            info.type = "以太网";
        } else if (interface.type() == QNetworkInterface::Wifi) {
            info.type = "Wi-Fi";
        } else if (interface.type() == QNetworkInterface::Loopback) {
            info.type = "回环";
        } else {
            info.type = "其他";
        }
        
        // 获取IP地址
        QList<QNetworkAddressEntry> addresses = interface.addressEntries();
        for (const QNetworkAddressEntry &entry : addresses) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                info.ipAddresses.append(entry.ip().toString());
                info.subnetMasks.append(entry.netmask().toString());
            }
        }
        
        if (info.ipAddresses.isEmpty()) {
            info.ipAddresses.append("未配置");
            info.subnetMasks.append("-");
        }
        
        networkCards.append(info);
    }
}

void MainWindow::populateNetworkTable()
{
    networkTable->setRowCount(networkCards.size());
    
    for (int i = 0; i < networkCards.size(); ++i) {
        const NetworkCardInfo &card = networkCards[i];
        
        networkTable->setItem(i, 0, new QTableWidgetItem(card.name));
        networkTable->setItem(i, 1, new QTableWidgetItem(card.displayName));
        networkTable->setItem(i, 2, new QTableWidgetItem(card.macAddress));
        networkTable->setItem(i, 3, new QTableWidgetItem(card.ipAddresses.join(", ")));
        networkTable->setItem(i, 4, new QTableWidgetItem(card.status));
        networkTable->setItem(i, 5, new QTableWidgetItem(card.type));
    }
    
    networkTable->resizeColumnsToContents();
}

void MainWindow::populateIPTable()
{
    if (currentSelectedCard < 0 || currentSelectedCard >= networkCards.size()) {
        ipTable->setRowCount(0);
        return;
    }
    
    const NetworkCardInfo &card = networkCards[currentSelectedCard];
    ipTable->setRowCount(card.ipAddresses.size());
    
    for (int i = 0; i < card.ipAddresses.size(); ++i) {
        ipTable->setItem(i, 0, new QTableWidgetItem(card.ipAddresses[i]));
        if (i < card.subnetMasks.size()) {
            ipTable->setItem(i, 1, new QTableWidgetItem(card.subnetMasks[i]));
        } else {
            ipTable->setItem(i, 1, new QTableWidgetItem("-"));
        }
        ipTable->setItem(i, 2, new QTableWidgetItem("IPv4"));
    }
    
    ipTable->resizeColumnsToContents();
}

void MainWindow::clearConfigPanel()
{
    selectedCardLabel->setText("<h3>请选择一个网卡</h3>");
    cardNameEdit->clear();
    macAddressLabel->setText("-");
    statusLabel->setText("-");
    typeLabel->setText("-");
    mtuLabel->setText("-");
    ipTable->setRowCount(0);
    
    addIpButton->setEnabled(false);
    removeIpButton->setEnabled(false);
    modifyIpButton->setEnabled(false);
    applyButton->setEnabled(false);
}

void MainWindow::refreshNetworkCards()
{
    loadNetworkCards();
    populateNetworkTable();
    clearConfigPanel();
    currentSelectedCard = -1;
    
    QMessageBox::information(this, "刷新完成", "网卡列表已更新");
}

void MainWindow::onNetworkCardSelectionChanged()
{
    int row = networkTable->currentRow();
    if (row < 0 || row >= networkCards.size()) {
        clearConfigPanel();
        currentSelectedCard = -1;
        return;
    }
    
    currentSelectedCard = row;
    const NetworkCardInfo &card = networkCards[row];
    
    selectedCardLabel->setText(QString("<h3>%1</h3>").arg(card.displayName));
    cardNameEdit->setText(card.name);
    macAddressLabel->setText(card.macAddress);
    statusLabel->setText(card.status);
    typeLabel->setText(card.type);
    mtuLabel->setText(QString::number(card.mtu));
    
    populateIPTable();
    
    // 启用按钮（除了回环接口）
    bool enableButtons = (card.type != "回环" && card.name != "lo0");
    addIpButton->setEnabled(enableButtons);
    removeIpButton->setEnabled(enableButtons);
    modifyIpButton->setEnabled(enableButtons);
    applyButton->setEnabled(enableButtons);
}

void MainWindow::addIPAddress()
{
    if (currentSelectedCard < 0) return;
    
    bool ok;
    QString ip = QInputDialog::getText(this, "添加IP地址", "请输入IP地址:", QLineEdit::Normal, "", &ok);
    if (!ok || ip.isEmpty()) return;
    
    QString netmask = QInputDialog::getText(this, "添加子网掩码", "请输入子网掩码:", QLineEdit::Normal, "255.255.255.0", &ok);
    if (!ok || netmask.isEmpty()) return;
    
    // 验证IP地址格式
    QHostAddress addr(ip);
    if (addr.isNull() || addr.protocol() != QAbstractSocket::IPv4Protocol) {
        QMessageBox::warning(this, "错误", "无效的IP地址格式");
        return;
    }
    
    // 验证子网掩码格式
    QHostAddress mask(netmask);
    if (mask.isNull()) {
        QMessageBox::warning(this, "错误", "无效的子网掩码格式");
        return;
    }
    
    // 添加到本地数据
    NetworkCardInfo &card = networkCards[currentSelectedCard];
    if (card.ipAddresses.contains("未配置")) {
        card.ipAddresses.clear();
        card.subnetMasks.clear();
    }
    card.ipAddresses.append(ip);
    card.subnetMasks.append(netmask);
    
    populateIPTable();
    populateNetworkTable();
    
    QMessageBox::information(this, "提示", "IP地址已添加到配置中，点击'应用更改'以生效");
}

void MainWindow::removeIPAddress()
{
    if (currentSelectedCard < 0) return;
    
    int row = ipTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "错误", "请选择要删除的IP地址");
        return;
    }
    
    QString ip = ipTable->item(row, 0)->text();
    if (ip == "未配置") {
        QMessageBox::warning(this, "错误", "无法删除未配置的项目");
        return;
    }
    
    int ret = QMessageBox::question(this, "确认删除", 
                                   QString("确定要删除IP地址 %1 吗？").arg(ip),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        NetworkCardInfo &card = networkCards[currentSelectedCard];
        card.ipAddresses.removeAt(row);
        if (row < card.subnetMasks.size()) {
            card.subnetMasks.removeAt(row);
        }
        
        if (card.ipAddresses.isEmpty()) {
            card.ipAddresses.append("未配置");
            card.subnetMasks.append("-");
        }
        
        populateIPTable();
        populateNetworkTable();
        
        QMessageBox::information(this, "提示", "IP地址已从配置中删除，点击'应用更改'以生效");
    }
}

void MainWindow::modifyIPAddress()
{
    if (currentSelectedCard < 0) return;
    
    int row = ipTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "错误", "请选择要修改的IP地址");
        return;
    }
    
    QString currentIp = ipTable->item(row, 0)->text();
    QString currentMask = ipTable->item(row, 1)->text();
    
    if (currentIp == "未配置") {
        QMessageBox::warning(this, "错误", "无法修改未配置的项目");
        return;
    }
    
    bool ok;
    QString newIp = QInputDialog::getText(this, "修改IP地址", "请输入新的IP地址:", QLineEdit::Normal, currentIp, &ok);
    if (!ok || newIp.isEmpty()) return;
    
    QString newMask = QInputDialog::getText(this, "修改子网掩码", "请输入新的子网掩码:", QLineEdit::Normal, currentMask, &ok);
    if (!ok || newMask.isEmpty()) return;
    
    // 验证IP地址格式
    QHostAddress addr(newIp);
    if (addr.isNull() || addr.protocol() != QAbstractSocket::IPv4Protocol) {
        QMessageBox::warning(this, "错误", "无效的IP地址格式");
        return;
    }
    
    // 验证子网掩码格式
    QHostAddress mask(newMask);
    if (mask.isNull()) {
        QMessageBox::warning(this, "错误", "无效的子网掩码格式");
        return;
    }
    
    // 更新本地数据
    NetworkCardInfo &card = networkCards[currentSelectedCard];
    card.ipAddresses[row] = newIp;
    if (row < card.subnetMasks.size()) {
        card.subnetMasks[row] = newMask;
    }
    
    populateIPTable();
    populateNetworkTable();
    
    QMessageBox::information(this, "提示", "IP地址已修改，点击'应用更改'以生效");
}

void MainWindow::applyNetworkChanges()
{
    if (currentSelectedCard < 0) return;
    
    int ret = QMessageBox::question(this, "确认应用", 
                                   "确定要应用网络配置更改吗？\n这可能会影响网络连接。",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    const NetworkCardInfo &card = networkCards[currentSelectedCard];
    QString interfaceName = card.name;
    
    // 在macOS上使用ifconfig命令应用更改
    QStringList commands;
    bool hasValidIPs = false;
    
    // 首先清除现有的IP配置（除了自动配置的）
    commands.append(QString("sudo ifconfig %1 delete").arg(interfaceName));
    
    // 应用新的IP配置
    for (int i = 0; i < card.ipAddresses.size(); ++i) {
        QString ip = card.ipAddresses[i];
        if (ip != "未配置") {
            QString mask = (i < card.subnetMasks.size()) ? card.subnetMasks[i] : "255.255.255.0";
            commands.append(QString("sudo ifconfig %1 %2 netmask %3 alias").arg(interfaceName, ip, mask));
            hasValidIPs = true;
        }
    }
    
    if (!hasValidIPs) {
        QMessageBox::warning(this, "错误", "没有有效的IP地址配置");
        return;
    }
    
    // 执行命令
    bool success = true;
    for (const QString &cmd : commands) {
        if (!executeNetworkCommand(cmd)) {
            success = false;
            break;
        }
    }
    
    if (success) {
        QMessageBox::information(this, "成功", "网络配置已应用成功！");
        refreshNetworkCards();
    } else {
        QMessageBox::critical(this, "失败", "应用网络配置失败！\n请检查权限或命令格式。");
    }
}

bool MainWindow::executeNetworkCommand(const QString &command)
{
    QProcess process;
    process.start("/bin/bash", QStringList() << "-c" << command);
    process.waitForFinished(5000);
    
    int exitCode = process.exitCode();
    if (exitCode != 0) {
        QString error = process.readAllStandardError();
        qDebug() << "Command failed:" << command << "Error:" << error;
        return false;
    }
    
    return true;
}

QString MainWindow::getNetmaskFromPrefix(int prefix)
{
    if (prefix < 0 || prefix > 32) return "255.255.255.0";
    
    quint32 mask = (0xFFFFFFFF << (32 - prefix));
    return QHostAddress(mask).toString();
}

int MainWindow::getPrefixFromNetmask(const QString &netmask)
{
    QHostAddress addr(netmask);
    if (addr.isNull()) return 24;
    
    quint32 mask = addr.toIPv4Address();
    int prefix = 0;
    while (mask & 0x80000000) {
        prefix++;
        mask <<= 1;
    }
    
    return prefix;
}
