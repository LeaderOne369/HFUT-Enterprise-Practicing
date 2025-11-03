#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_bookmarkManager(nullptr)
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setWindowTitle("精美浏览器");
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    resize(1200, 800);
    
    // 先创建UI组件
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    
    // 加载主页
    if (m_webView) {
        m_webView->load(QUrl("https://www.baidu.com"));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_bookmarkManager) {
        delete m_bookmarkManager;
    }
}

void MainWindow::setupUI()
{
    // 创建中央部件
    QWidget *centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建导航栏布局
    QHBoxLayout *navLayout = new QHBoxLayout;
    navLayout->setContentsMargins(5, 5, 5, 5);
    navLayout->setSpacing(5);
    
    // 导航按钮
    m_backButton = new QPushButton;
    m_backButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowBack));
    m_backButton->setToolTip("后退");
    m_backButton->setFixedSize(32, 32);
    
    m_forwardButton = new QPushButton;
    m_forwardButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowForward));
    m_forwardButton->setToolTip("前进");
    m_forwardButton->setFixedSize(32, 32);
    
    m_reloadButton = new QPushButton;
    m_reloadButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    m_reloadButton->setToolTip("刷新");
    m_reloadButton->setFixedSize(32, 32);
    
    m_homeButton = new QPushButton;
    m_homeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon));
    m_homeButton->setToolTip("主页");
    m_homeButton->setFixedSize(32, 32);
    
    // 地址栏
    m_urlEdit = new QLineEdit;
    m_urlEdit->setPlaceholderText("请输入网址...");
    m_urlEdit->setStyleSheet("QLineEdit { border: 2px solid #ddd; border-radius: 4px; padding: 6px; font-size: 14px; }");
    
    // 转到按钮
    m_goButton = new QPushButton("转到");
    m_goButton->setFixedSize(60, 32);
    m_goButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 4px; font-weight: bold; }");
    
    // 书签按钮
    m_bookmarkButton = new QPushButton;
    m_bookmarkButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_bookmarkButton->setToolTip("书签");
    m_bookmarkButton->setFixedSize(32, 32);
    
    // 添加到导航布局
    navLayout->addWidget(m_backButton);
    navLayout->addWidget(m_forwardButton);
    navLayout->addWidget(m_reloadButton);
    navLayout->addWidget(m_homeButton);
    navLayout->addWidget(m_urlEdit);
    navLayout->addWidget(m_goButton);
    navLayout->addWidget(m_bookmarkButton);
    
    // 创建网页视图 - 暂时注释掉
    m_webView = new QWebEngineView;
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal);
    m_splitter->addWidget(m_webView);
    m_splitter->setSizes({1000});
    
    // 添加到主布局
    mainLayout->addLayout(navLayout);
    mainLayout->addWidget(m_splitter);
    
    // 连接信号槽
    connect(m_backButton, &QPushButton::clicked, this, &MainWindow::goBack);
    connect(m_forwardButton, &QPushButton::clicked, this, &MainWindow::goForward);
    connect(m_reloadButton, &QPushButton::clicked, this, &MainWindow::reload);
    connect(m_homeButton, &QPushButton::clicked, this, &MainWindow::goHome);
    connect(m_goButton, &QPushButton::clicked, this, &MainWindow::navigateToUrl);
    connect(m_bookmarkButton, &QPushButton::clicked, this, &MainWindow::showBookmarks);
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
    
    // 暂时注释掉WebEngine相关的信号槽连接
    connect(m_webView, &QWebEngineView::urlChanged, this, &MainWindow::updateUrlBar);
    connect(m_webView, &QWebEngineView::loadProgress, this, &MainWindow::updateLoadProgress);
    connect(m_webView, &QWebEngineView::loadFinished, this, &MainWindow::finishLoading);
    
    // 初始状态
    updateNavigationButtons();
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    
    m_printPreviewAction = new QAction("打印预览(&P)", this);
    m_printPreviewAction->setShortcut(QKeySequence::Print);
    m_printAction = new QAction("打印(&R)", this);
    m_printAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    m_exitAction = new QAction("退出(&X)", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    
    fileMenu->addAction(m_printPreviewAction);
    fileMenu->addAction(m_printAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);
    
    // 书签菜单
    QMenu *bookmarkMenu = menuBar()->addMenu("书签(&B)");
    
    m_bookmarkAction = new QAction("书签管理器(&M)", this);
    m_bookmarkAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    m_addBookmarkAction = new QAction("添加书签(&A)", this);
    m_addBookmarkAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    
    bookmarkMenu->addAction(m_addBookmarkAction);
    bookmarkMenu->addAction(m_bookmarkAction);
    
    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    m_aboutAction = new QAction("关于(&A)", this);
    helpMenu->addAction(m_aboutAction);
    
    // 连接信号槽
    connect(m_printPreviewAction, &QAction::triggered, this, &MainWindow::printPreview);
    connect(m_printAction, &QAction::triggered, this, &MainWindow::printPage);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    connect(m_bookmarkAction, &QAction::triggered, this, &MainWindow::showBookmarks);
    connect(m_addBookmarkAction, &QAction::triggered, this, &MainWindow::addCurrentBookmark);
    connect(m_aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于", "精美浏览器 v1.0\n基于Qt WebEngine开发\n支持网页浏览、书签管理、页面打印等功能");
    });
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(false);
    
    toolBar->addAction(QApplication::style()->standardIcon(QStyle::SP_ArrowBack), "后退", this, &MainWindow::goBack);
    toolBar->addAction(QApplication::style()->standardIcon(QStyle::SP_ArrowForward), "前进", this, &MainWindow::goForward);
    toolBar->addAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload), "刷新", this, &MainWindow::reload);
    toolBar->addAction(QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon), "主页", this, &MainWindow::goHome);
    toolBar->addSeparator();
    toolBar->addAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView), "书签", this, &MainWindow::showBookmarks);
    toolBar->addAction("添加书签", this, &MainWindow::addCurrentBookmark);
}

void MainWindow::setupStatusBar()
{
    // 进度条
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    
    statusBar()->addPermanentWidget(m_progressBar);
    statusBar()->showMessage("就绪");
}

void MainWindow::navigateToUrl()
{
    QString urlString = m_urlEdit->text().trimmed();
    if (urlString.isEmpty() || !m_webView) return;
    
    // 如果不包含协议，添加http://
    if (!urlString.startsWith("http://") && !urlString.startsWith("https://")) {
        urlString = "http://" + urlString;
    }
    
    QUrl url(urlString);
    if (url.isValid()) {
        m_webView->load(url);
    }
}

void MainWindow::goBack()
{
    if (m_webView) {
        m_webView->back();
    }
}

void MainWindow::goForward()
{
    if (m_webView) {
        m_webView->forward();
    }
}

void MainWindow::reload()
{
    if (m_webView) {
        m_webView->reload();
    }
}

void MainWindow::goHome()
{
    if (m_webView) {
        m_webView->load(QUrl("https://www.baidu.com"));
    }
}

void MainWindow::updateUrlBar(const QUrl &url)
{
    m_urlEdit->setText(url.toString());
    updateNavigationButtons();
}

void MainWindow::updateLoadProgress(int progress)
{
    if (progress < 100) {
        m_progressBar->setValue(progress);
        m_progressBar->setVisible(true);
        statusBar()->showMessage(QString("加载中... %1%").arg(progress));
    }
}

void MainWindow::finishLoading(bool success)
{
    m_progressBar->setVisible(false);
    if (success) {
        statusBar()->showMessage("页面加载完成", 2000);
    } else {
        statusBar()->showMessage("页面加载失败", 2000);
    }
    updateNavigationButtons();
}

void MainWindow::updateNavigationButtons()
{
    if (m_webView && m_webView->history()) {
        m_backButton->setEnabled(m_webView->history()->canGoBack());
        m_forwardButton->setEnabled(m_webView->history()->canGoForward());
    } else {
        m_backButton->setEnabled(false);
        m_forwardButton->setEnabled(false);
    }
}

void MainWindow::showBookmarks()
{
    if (!m_bookmarkManager) {
        m_bookmarkManager = new BookmarkManager(this);
        connect(m_bookmarkManager, &BookmarkManager::bookmarkClicked, 
                this, &MainWindow::openBookmarkUrl);
    }
    m_bookmarkManager->show();
    m_bookmarkManager->raise();
    m_bookmarkManager->activateWindow();
}

void MainWindow::addCurrentBookmark()
{
    if (!m_webView) {
        QMessageBox::information(this, "提示", "浏览器组件未初始化完成，请稍后再试。");
        return;
    }
    
    QString title = m_webView->title();
    QUrl url = m_webView->url();
    
    if (title.isEmpty()) {
        title = url.toString();
    }
    
    if (!m_bookmarkManager) {
        m_bookmarkManager = new BookmarkManager(this);
        connect(m_bookmarkManager, &BookmarkManager::bookmarkClicked, 
                this, &MainWindow::openBookmarkUrl);
    }
    
    m_bookmarkManager->addBookmark(title, url);
    statusBar()->showMessage("书签已添加", 2000);
}

void MainWindow::openBookmarkUrl(const QUrl &url)
{
    if (m_webView) {
        m_webView->load(url);
    }
}

void MainWindow::printPage()
{
    QMessageBox::information(this, "打印功能", 
        "要打印当前页面，请使用以下方法：\n"
        "1. 按 Ctrl+P 使用浏览器内置打印功能\n"
        "2. 右键点击页面选择打印\n"
        "3. 通过菜单栏选择打印选项");
}

void MainWindow::printPreview()
{
    QMessageBox::information(this, "打印预览", 
        "要预览打印效果，请使用以下方法：\n"
        "1. 按 Ctrl+P 打开打印对话框\n"
        "2. 在打印对话框中选择预览选项\n"
        "3. 使用浏览器内置的打印预览功能");
}
