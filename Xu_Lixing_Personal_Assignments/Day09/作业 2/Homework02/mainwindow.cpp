#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableView(nullptr)
    , m_downloadModel(nullptr)
    , m_progressDelegate(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    
    // 添加一些示例数据
    m_downloadModel->addDownload("大文件1.zip", 100 * 1024 * 1024); // 100MB
    m_downloadModel->addDownload("视频文件.mp4", 500 * 1024 * 1024); // 500MB
    m_downloadModel->addDownload("软件包.exe", 50 * 1024 * 1024);   // 50MB
    m_downloadModel->addDownload("文档.pdf", 10 * 1024 * 1024);     // 10MB
}

MainWindow::~MainWindow()
{
    if (m_downloadModel) {
        m_downloadModel->stopSimulation();
    }
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("文件下载管理器");
    setMinimumSize(800, 600);
    
    // 创建中央窗口部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 创建表格视图
    m_tableView = new QTableView(this);
    m_downloadModel = new DownloadModel(this);
    m_progressDelegate = new ProgressBarDelegate(this);
    
    m_tableView->setModel(m_downloadModel);
    m_tableView->setItemDelegateForColumn(DownloadModel::ProgressColumn, m_progressDelegate);
    
    // 设置表格样式
    m_tableView->setStyleSheet(
        "QTableView {"
        "    gridline-color: #e0e0e0;"
        "    background-color: white;"
        "    alternate-background-color: #f5f5f5;"
        "    selection-background-color: #4CAF50;"
        "    border: 1px solid #ddd;"
        "    border-radius: 5px;"
        "}"
        "QTableView::item {"
        "    padding: 8px;"
        "    border: none;"
        "}"
        "QTableView::item:selected {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "}"
        "QHeaderView::section {"
        "    background-color: #f8f9fa;"
        "    padding: 8px;"
        "    border: 1px solid #dee2e6;"
        "    font-weight: bold;"
        "    color: #495057;"
        "}"
    );
    
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setDefaultSectionSize(40);
    m_tableView->verticalHeader()->hide();
    
    // 设置列宽
    m_tableView->setColumnWidth(DownloadModel::FileNameColumn, 200);
    m_tableView->setColumnWidth(DownloadModel::FileSizeColumn, 100);
    m_tableView->setColumnWidth(DownloadModel::ProgressColumn, 150);
    m_tableView->setColumnWidth(DownloadModel::SpeedColumn, 120);
    m_tableView->setColumnWidth(DownloadModel::RemainingTimeColumn, 120);
    m_tableView->setColumnWidth(DownloadModel::StatusColumn, 100);
    
    mainLayout->addWidget(m_tableView);
    
    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    // 创建按钮
    m_addButton = new QPushButton("添加下载", this);
    m_removeButton = new QPushButton("删除选中", this);
    m_clearButton = new QPushButton("清空所有", this);
    m_startButton = new QPushButton("开始下载", this);
    m_stopButton = new QPushButton("停止下载", this);
    
    // 设置按钮样式
    QString buttonStyle = 
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #004085;"
        "}";
    
    m_addButton->setStyleSheet(buttonStyle);
    m_removeButton->setStyleSheet(buttonStyle.replace("#007bff", "#28a745").replace("#0056b3", "#1e7e34").replace("#004085", "#155724"));
    m_clearButton->setStyleSheet(buttonStyle.replace("#28a745", "#dc3545").replace("#1e7e34", "#c82333").replace("#155724", "#721c24"));
    m_startButton->setStyleSheet(buttonStyle.replace("#dc3545", "#ffc107").replace("#c82333", "#e0a800").replace("#721c24", "#b08000"));
    m_stopButton->setStyleSheet(buttonStyle.replace("#ffc107", "#6c757d").replace("#e0a800", "#545b62").replace("#b08000", "#495057"));
    
    // 添加按钮到布局
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 设置窗口样式
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f8f9fa;"
        "}"
    );
}

void MainWindow::setupConnections()
{
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::addDownload);
    connect(m_removeButton, &QPushButton::clicked, this, &MainWindow::removeDownload);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearAllDownloads);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::startDownloads);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopDownloads);
}

void MainWindow::addDownload()
{
    bool ok;
    QString fileName = QInputDialog::getText(this, "添加下载", "请输入文件名:", 
                                           QLineEdit::Normal, "新文件.zip", &ok);
    if (!ok || fileName.isEmpty())
        return;
    
    double fileSizeMB = QInputDialog::getDouble(this, "添加下载", "请输入文件大小(MB):", 
                                              10.0, 0.1, 10000.0, 2, &ok);
    if (!ok)
        return;
    
    qint64 fileSize = static_cast<qint64>(fileSizeMB * 1024 * 1024);
    m_downloadModel->addDownload(fileName, fileSize);
}

void MainWindow::removeDownload()
{
    QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要删除的下载项目。");
        return;
    }
    
    // 从后往前删除，避免索引变化
    QList<int> rows;
    for (const QModelIndex &index : selection) {
        rows.append(index.row());
    }
    std::sort(rows.rbegin(), rows.rend());
    
    for (int row : rows) {
        m_downloadModel->removeDownload(row);
    }
}

void MainWindow::clearAllDownloads()
{
    int ret = QMessageBox::question(this, "确认", "确定要清空所有下载项目吗？",
                                  QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_downloadModel->stopSimulation();
        m_downloadModel->clearAll();
    }
}

void MainWindow::startDownloads()
{
    m_downloadModel->startSimulation();
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
}

void MainWindow::stopDownloads()
{
    m_downloadModel->stopSimulation();
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
}
