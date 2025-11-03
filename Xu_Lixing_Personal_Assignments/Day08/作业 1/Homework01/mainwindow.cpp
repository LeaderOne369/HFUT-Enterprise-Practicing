#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isDragging(false)
    , backgroundColor(QColor(240, 240, 240))
    , isMaximized(false)
    , currentTheme("默认主题")
{
    ui->setupUi(this);
    
    // 隐藏默认的标题栏
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    
    setupUI();
    setupCustomTitleBar();
    setupTrayIcon();
    applyTheme(currentTheme);
    
    resize(900, 700);
    setMinimumSize(600, 400);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);
    
    // 主题设置组
    themeGroup = new QGroupBox("外观设置", this);
    QVBoxLayout *themeLayout = new QVBoxLayout(themeGroup);
    
    // 主题选择器
    QHBoxLayout *themeSelectLayout = new QHBoxLayout();
    QLabel *themeLabel = new QLabel("主题风格:", this);
    themeComboBox = new QComboBox(this);
    themeComboBox->addItems({"默认主题", "深色主题", "蓝色主题", "绿色主题", "紫色主题"});
    connect(themeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::onThemeChanged);
    
    themeSelectLayout->addWidget(themeLabel);
    themeSelectLayout->addWidget(themeComboBox);
    themeSelectLayout->addStretch();
    
    // 背景设置
    QHBoxLayout *backgroundLayout = new QHBoxLayout();
    backgroundColorButton = new QPushButton("设置背景颜色", this);
    backgroundImageButton = new QPushButton("设置背景图片", this);
    connect(backgroundColorButton, &QPushButton::clicked, this, &MainWindow::onBackgroundColorChanged);
    connect(backgroundImageButton, &QPushButton::clicked, this, &MainWindow::onBackgroundImageChanged);
    
    backgroundLayout->addWidget(backgroundColorButton);
    backgroundLayout->addWidget(backgroundImageButton);
    backgroundLayout->addStretch();
    
    // 功能按钮
    QHBoxLayout *functionLayout = new QHBoxLayout();
    fullScreenButton = new QPushButton("切换全屏", this);
    enableTrayCheckBox = new QCheckBox("启用系统托盘", this);
    enableTrayCheckBox->setChecked(true);
    connect(fullScreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);
    
    functionLayout->addWidget(fullScreenButton);
    functionLayout->addWidget(enableTrayCheckBox);
    functionLayout->addStretch();
    
    themeLayout->addLayout(themeSelectLayout);
    themeLayout->addLayout(backgroundLayout);
    themeLayout->addLayout(functionLayout);
    
    // 示例内容区域
    QGroupBox *contentGroup = new QGroupBox("功能演示", this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentGroup);
    
    QLabel *welcomeLabel = new QLabel("欢迎使用自定义窗口应用程序！", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setFont(QFont("微软雅黑", 16, QFont::Bold));
    
    QLabel *featuresLabel = new QLabel(
        "功能特性：\n"
        "• 自定义标题栏和窗口控制按钮\n"
        "• 多种主题风格动态切换\n"
        "• 背景颜色和图片自定义\n"
        "• 支持全屏显示和窗口大小调节\n"
        "• 系统托盘最小化功能\n"
        "• 响应式界面布局", this);
    featuresLabel->setWordWrap(true);
    
    contentLayout->addWidget(welcomeLabel);
    contentLayout->addWidget(featuresLabel);
    contentLayout->addStretch();
    
    mainLayout->addWidget(themeGroup);
    mainLayout->addWidget(contentGroup);
    mainLayout->addStretch();
}

void MainWindow::setupCustomTitleBar()
{
    titleBar = new QWidget(this);
    titleBar->setFixedHeight(40);
    titleBar->setObjectName("titleBar");
    
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(10, 0, 10, 0);
    titleLayout->setSpacing(10);
    
    // 标题
    titleLabel = new QLabel("精美QT应用程序", titleBar);
    titleLabel->setFont(QFont("微软雅黑", 12, QFont::Bold));
    titleLabel->setObjectName("titleLabel");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    
    // 窗口控制按钮
    minimizeButton = new QPushButton("−", titleBar);
    maximizeButton = new QPushButton("□", titleBar);
    closeButton = new QPushButton("×", titleBar);
    
    minimizeButton->setFixedSize(30, 30);
    maximizeButton->setFixedSize(30, 30);
    closeButton->setFixedSize(30, 30);
    
    minimizeButton->setObjectName("minimizeButton");
    maximizeButton->setObjectName("maximizeButton");
    closeButton->setObjectName("closeButton");
    
    connect(minimizeButton, &QPushButton::clicked, this, &MainWindow::onMinimizeClicked);
    connect(maximizeButton, &QPushButton::clicked, this, &MainWindow::onMaximizeClicked);
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
    
    titleLayout->addWidget(minimizeButton);
    titleLayout->addWidget(maximizeButton);
    titleLayout->addWidget(closeButton);
    
    // 将标题栏添加到主布局的顶部
    QVBoxLayout *windowLayout = new QVBoxLayout();
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->setSpacing(0);
    windowLayout->addWidget(titleBar);
    windowLayout->addWidget(centralWidget);
    
    QWidget *windowWidget = new QWidget(this);
    windowWidget->setLayout(windowLayout);
    setCentralWidget(windowWidget);
}

void MainWindow::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(this, "系统托盘", "系统不支持托盘功能");
        return;
    }
    
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icons/app.png"));
    trayIcon->setToolTip("精美QT应用程序");
    
    // 创建托盘菜单
    trayMenu = new QMenu(this);
    showAction = new QAction("显示窗口", this);
    quitAction = new QAction("退出程序", this);
    
    connect(showAction, &QAction::triggered, this, &MainWindow::showFromTray);
    connect(quitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    
    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

void MainWindow::applyTheme(const QString &themeName)
{
    currentTheme = themeName;
    QString styleSheet;
    
    if (themeName == "深色主题") {
        styleSheet = R"(
            QMainWindow {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            #titleBar {
                background-color: #1e1e1e;
                border-bottom: 1px solid #404040;
            }
            #titleLabel {
                color: #ffffff;
            }
            QPushButton {
                background-color: #404040;
                color: #ffffff;
                border: 1px solid #606060;
                border-radius: 5px;
                padding: 8px 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #505050;
            }
            QPushButton:pressed {
                background-color: #303030;
            }
            #minimizeButton, #maximizeButton, #closeButton {
                background-color: transparent;
                border: none;
                font-size: 16px;
                font-weight: bold;
            }
            #minimizeButton:hover, #maximizeButton:hover {
                background-color: #404040;
            }
            #closeButton:hover {
                background-color: #e74c3c;
            }
            QGroupBox {
                background-color: #3a3a3a;
                border: 2px solid #505050;
                border-radius: 8px;
                margin-top: 1ex;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
                color: #ffffff;
            }
            QComboBox {
                background-color: #404040;
                color: #ffffff;
                border: 1px solid #606060;
                border-radius: 4px;
                padding: 5px;
            }
            QCheckBox {
                color: #ffffff;
            }
            QLabel {
                color: #ffffff;
            }
        )";
    } else if (themeName == "蓝色主题") {
        styleSheet = R"(
            QMainWindow {
                background-color: #e6f3ff;
                color: #1a1a1a;
            }
            #titleBar {
                background-color: #0066cc;
                border-bottom: 1px solid #004499;
            }
            #titleLabel {
                color: #ffffff;
            }
            QPushButton {
                background-color: #3399ff;
                color: #ffffff;
                border: 1px solid #0066cc;
                border-radius: 5px;
                padding: 8px 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #66b3ff;
            }
            QPushButton:pressed {
                background-color: #0066cc;
            }
            #minimizeButton, #maximizeButton, #closeButton {
                background-color: transparent;
                border: none;
                font-size: 16px;
                font-weight: bold;
                color: #ffffff;
            }
            #minimizeButton:hover, #maximizeButton:hover {
                background-color: rgba(255,255,255,0.2);
            }
            #closeButton:hover {
                background-color: #e74c3c;
            }
            QGroupBox {
                background-color: #ffffff;
                border: 2px solid #3399ff;
                border-radius: 8px;
                margin-top: 1ex;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
                color: #0066cc;
            }
            QComboBox {
                background-color: #ffffff;
                border: 1px solid #3399ff;
                border-radius: 4px;
                padding: 5px;
            }
        )";
    } else if (themeName == "绿色主题") {
        styleSheet = R"(
            QMainWindow {
                background-color: #f0fff0;
                color: #1a1a1a;
            }
            #titleBar {
                background-color: #228b22;
                border-bottom: 1px solid #006400;
            }
            #titleLabel {
                color: #ffffff;
            }
            QPushButton {
                background-color: #32cd32;
                color: #ffffff;
                border: 1px solid #228b22;
                border-radius: 5px;
                padding: 8px 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #98fb98;
                color: #1a1a1a;
            }
            QPushButton:pressed {
                background-color: #228b22;
            }
            #minimizeButton, #maximizeButton, #closeButton {
                background-color: transparent;
                border: none;
                font-size: 16px;
                font-weight: bold;
                color: #ffffff;
            }
            #minimizeButton:hover, #maximizeButton:hover {
                background-color: rgba(255,255,255,0.2);
            }
            #closeButton:hover {
                background-color: #e74c3c;
            }
            QGroupBox {
                background-color: #ffffff;
                border: 2px solid #32cd32;
                border-radius: 8px;
                margin-top: 1ex;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
                color: #228b22;
            }
            QComboBox {
                background-color: #ffffff;
                border: 1px solid #32cd32;
                border-radius: 4px;
                padding: 5px;
            }
        )";
    } else if (themeName == "紫色主题") {
        styleSheet = R"(
            QMainWindow {
                background-color: #f8f0ff;
                color: #1a1a1a;
            }
            #titleBar {
                background-color: #8a2be2;
                border-bottom: 1px solid #4b0082;
            }
            #titleLabel {
                color: #ffffff;
            }
            QPushButton {
                background-color: #da70d6;
                color: #ffffff;
                border: 1px solid #8a2be2;
                border-radius: 5px;
                padding: 8px 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #dda0dd;
            }
            QPushButton:pressed {
                background-color: #8a2be2;
            }
            #minimizeButton, #maximizeButton, #closeButton {
                background-color: transparent;
                border: none;
                font-size: 16px;
                font-weight: bold;
                color: #ffffff;
            }
            #minimizeButton:hover, #maximizeButton:hover {
                background-color: rgba(255,255,255,0.2);
            }
            #closeButton:hover {
                background-color: #e74c3c;
            }
            QGroupBox {
                background-color: #ffffff;
                border: 2px solid #da70d6;
                border-radius: 8px;
                margin-top: 1ex;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
                color: #8a2be2;
            }
            QComboBox {
                background-color: #ffffff;
                border: 1px solid #da70d6;
                border-radius: 4px;
                padding: 5px;
            }
        )";
    } else { // 默认主题
        styleSheet = R"(
            QMainWindow {
                background-color: #f5f5f5;
                color: #1a1a1a;
            }
            #titleBar {
                background-color: #4a90e2;
                border-bottom: 1px solid #357abd;
            }
            #titleLabel {
                color: #ffffff;
            }
            QPushButton {
                background-color: #4a90e2;
                color: #ffffff;
                border: 1px solid #357abd;
                border-radius: 5px;
                padding: 8px 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #357abd;
            }
            QPushButton:pressed {
                background-color: #2c5d8f;
            }
            #minimizeButton, #maximizeButton, #closeButton {
                background-color: transparent;
                border: none;
                font-size: 16px;
                font-weight: bold;
                color: #ffffff;
            }
            #minimizeButton:hover, #maximizeButton:hover {
                background-color: rgba(255,255,255,0.2);
            }
            #closeButton:hover {
                background-color: #e74c3c;
            }
            QGroupBox {
                background-color: #ffffff;
                border: 2px solid #4a90e2;
                border-radius: 8px;
                margin-top: 1ex;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
                color: #4a90e2;
            }
            QComboBox {
                background-color: #ffffff;
                border: 1px solid #4a90e2;
                border-radius: 4px;
                padding: 5px;
            }
        )";
    }
    
    setStyleSheet(styleSheet);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && titleBar && titleBar->geometry().contains(event->pos())) {
        isDragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && isDragging) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    if (!backgroundImagePath.isEmpty()) {
        QPixmap pixmap(backgroundImagePath);
        if (!pixmap.isNull()) {
            painter.drawPixmap(rect(), pixmap);
        }
    } else {
        painter.fillRect(rect(), backgroundColor);
    }
    
    QMainWindow::paintEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (enableTrayCheckBox->isChecked() && trayIcon && trayIcon->isVisible()) {
        QMessageBox::information(this, "系统托盘",
                                 "程序将最小化到系统托盘。要退出程序，请右键点击托盘图标并选择退出。");
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::onMinimizeClicked()
{
    showMinimized();
}

void MainWindow::onMaximizeClicked()
{
    if (isMaximized) {
        showNormal();
        maximizeButton->setText("□");
        isMaximized = false;
    } else {
        showMaximized();
        maximizeButton->setText("❐");
        isMaximized = true;
    }
}

void MainWindow::onCloseClicked()
{
    close();
}

void MainWindow::onThemeChanged()
{
    applyTheme(themeComboBox->currentText());
}

void MainWindow::onBackgroundColorChanged()
{
    QColor color = QColorDialog::getColor(backgroundColor, this, "选择背景颜色");
    if (color.isValid()) {
        setBackgroundColor(color);
    }
}

void MainWindow::onBackgroundImageChanged()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                   "选择背景图片",
                                                   "",
                                                   "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!fileName.isEmpty()) {
        setBackgroundImage(fileName);
    }
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        fullScreenButton->setText("切换全屏");
    } else {
        showFullScreen();
        fullScreenButton->setText("退出全屏");
    }
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        showFromTray();
        break;
    default:
        break;
    }
}

void MainWindow::showFromTray()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}

void MainWindow::setBackgroundColor(const QColor &color)
{
    backgroundColor = color;
    backgroundImagePath.clear();
    update();
}

void MainWindow::setBackgroundImage(const QString &imagePath)
{
    backgroundImagePath = imagePath;
    update();
}
