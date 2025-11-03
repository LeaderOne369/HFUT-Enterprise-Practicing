#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <QRandomGenerator>
#include <QStringConverter>

// ================== ProgressThread 实现 ==================
ProgressThread::ProgressThread(QObject *parent)
    : QThread(parent), m_min(0), m_max(100), m_stopped(false)
{
}

void ProgressThread::setProgressRange(int min, int max)
{
    QMutexLocker locker(&m_mutex);
    m_min = min;
    m_max = max;
}

void ProgressThread::stopProgress()
{
    QMutexLocker locker(&m_mutex);
    m_stopped = true;
}

void ProgressThread::run()
{
    m_stopped = false;
    for (int i = m_min; i <= m_max && !m_stopped; ++i) {
        msleep(50); // 模拟工作
        emit progressUpdated(i);
    }
    
    if (!m_stopped) {
        emit progressFinished();
    }
}

// ================== FileWriterThread 实现 ==================
FileWriterThread::FileWriterThread(QObject *parent)
    : QThread(parent)
{
}

void FileWriterThread::setFileData(const QString &fileName, const QStringList &data)
{
    QMutexLocker locker(&m_mutex);
    m_fileName = fileName;
    m_data = data;
}

void FileWriterThread::run()
{
    if (m_fileName.isEmpty() || m_data.isEmpty()) {
        emit fileWriteError("文件名或数据为空");
        return;
    }
    
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit fileWriteError("无法打开文件进行写入: " + file.errorString());
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    int totalLines = m_data.size();
    for (int i = 0; i < totalLines; ++i) {
        out << m_data[i] << "\n";
        
        // 模拟写入延迟
        msleep(100);
        
        int percentage = (i + 1) * 100 / totalLines;
        emit fileWriteProgress(percentage);
    }
    
    file.close();
    emit fileWriteFinished("文件写入完成: " + m_fileName);
}

// ================== DataProcessThread 实现 ==================
DataProcessThread::DataProcessThread(QObject *parent)
    : QThread(parent)
{
}

void DataProcessThread::setProcessData(const QStringList &data)
{
    QMutexLocker locker(&m_mutex);
    m_inputData = data;
}

void DataProcessThread::run()
{
    if (m_inputData.isEmpty()) {
        emit dataProcessed("输入数据为空");
        emit processFinished();
        return;
    }
    
    QStringList processedData;
    int totalItems = m_inputData.size();
    
    for (int i = 0; i < totalItems; ++i) {
        QString item = m_inputData[i];
        
        // 模拟数据处理：转换为大写、添加序号、计算长度
        QString processed = QString("[%1] %2 (长度: %3)")
                               .arg(i + 1, 3, 10, QChar('0'))
                               .arg(item.toUpper())
                               .arg(item.length());
        
        processedData.append(processed);
        
        // 模拟处理延迟
        msleep(150);
        
        int percentage = (i + 1) * 100 / totalItems;
        emit processProgress(percentage);
    }
    
    QString result = processedData.join("\n");
    emit dataProcessed(result);
    emit processFinished();
}

// ================== MainWindow 实现 ==================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_progressThread(nullptr)
    , m_fileWriterThread(nullptr)
    , m_dataProcessThread(nullptr)
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setWindowTitle("多线程任务管理器 - Qt实训作业");
    setMinimumSize(900, 700);
    resize(1200, 800);
    
    // 创建线程对象
    m_progressThread = new ProgressThread(this);
    m_fileWriterThread = new FileWriterThread(this);
    m_dataProcessThread = new DataProcessThread(this);
    
    // 状态更新定时器
    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(1000);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateThreadStatus);
    m_statusTimer->start();
    
    setupUI();
    setupConnections();
    
    // 设置初始状态
    updateThreadStatus();
}

MainWindow::~MainWindow()
{
    // 停止所有线程
    if (m_progressThread && m_progressThread->isRunning()) {
        m_progressThread->stopProgress();
        m_progressThread->quit();
        m_progressThread->wait(3000);
    }
    
    if (m_fileWriterThread && m_fileWriterThread->isRunning()) {
        m_fileWriterThread->quit();
        m_fileWriterThread->wait(3000);
    }
    
    if (m_dataProcessThread && m_dataProcessThread->isRunning()) {
        m_dataProcessThread->quit();
        m_dataProcessThread->wait(3000);
    }
    
    delete ui;
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 标题
    QLabel *titleLabel = new QLabel("多线程任务管理器", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #2c3e50; margin: 10px 0; }");
    mainLayout->addWidget(titleLabel);
    
    // 整体状态标签
    m_overallStatus = new QLabel("系统就绪", this);
    m_overallStatus->setAlignment(Qt::AlignCenter);
    m_overallStatus->setStyleSheet("QLabel { font-size: 14px; color: #27ae60; padding: 5px; background-color: #ecf0f1; border-radius: 3px; }");
    mainLayout->addWidget(m_overallStatus);
    
    // 创建三个功能组
    QHBoxLayout *groupsLayout = new QHBoxLayout();
    groupsLayout->setSpacing(15);
    
    // ===== 进度更新组 =====
    m_progressGroup = new QGroupBox("进度更新线程", this);
    m_progressGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *progressLayout = new QVBoxLayout(m_progressGroup);
    
    QLabel *progressLabel = new QLabel("设置进度范围:", this);
    progressLayout->addWidget(progressLabel);
    
    QHBoxLayout *progressRangeLayout = new QHBoxLayout();
    progressRangeLayout->addWidget(new QLabel("最大值:", this));
    m_progressMaxSpin = new QSpinBox(this);
    m_progressMaxSpin->setRange(10, 1000);
    m_progressMaxSpin->setValue(100);
    m_progressMaxSpin->setStyleSheet("QSpinBox { padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px; }");
    progressRangeLayout->addWidget(m_progressMaxSpin);
    progressRangeLayout->addStretch();
    progressLayout->addLayout(progressRangeLayout);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setStyleSheet("QProgressBar { border: 2px solid #bdc3c7; border-radius: 5px; text-align: center; } QProgressBar::chunk { background-color: #3498db; border-radius: 3px; }");
    progressLayout->addWidget(m_progressBar);
    
    QHBoxLayout *progressBtnLayout = new QHBoxLayout();
    m_startProgressBtn = new QPushButton("开始进度", this);
    m_startProgressBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 8px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #229954; } QPushButton:disabled { background-color: #95a5a6; }");
    m_stopProgressBtn = new QPushButton("停止进度", this);
    m_stopProgressBtn->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; padding: 8px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #c0392b; } QPushButton:disabled { background-color: #95a5a6; }");
    progressBtnLayout->addWidget(m_startProgressBtn);
    progressBtnLayout->addWidget(m_stopProgressBtn);
    progressLayout->addLayout(progressBtnLayout);
    
    m_progressStatus = new QLabel("就绪", this);
    m_progressStatus->setStyleSheet("QLabel { color: #7f8c8d; font-style: italic; }");
    progressLayout->addWidget(m_progressStatus);
    
    progressLayout->addStretch();
    groupsLayout->addWidget(m_progressGroup);
    
    // ===== 文件写入组 =====
    m_fileGroup = new QGroupBox("文件写入线程", this);
    m_fileGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *fileLayout = new QVBoxLayout(m_fileGroup);
    
    QLabel *fileLabel = new QLabel("选择输出文件:", this);
    fileLayout->addWidget(fileLabel);
    
    QHBoxLayout *filePathLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setPlaceholderText("点击选择文件路径...");
    m_filePathEdit->setStyleSheet("QLineEdit { padding: 5px; border: 1px solid #bdc3c7; border-radius: 3px; }");
    m_selectFileBtn = new QPushButton("浏览", this);
    m_selectFileBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px 15px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #2980b9; }");
    filePathLayout->addWidget(m_filePathEdit);
    filePathLayout->addWidget(m_selectFileBtn);
    fileLayout->addLayout(filePathLayout);
    
    QLabel *dataLabel = new QLabel("输入要写入的数据 (每行一项):", this);
    fileLayout->addWidget(dataLabel);
    
    m_fileDataEdit = new QTextEdit(this);
    m_fileDataEdit->setMaximumHeight(120);
    m_fileDataEdit->setPlaceholderText("输入数据，每行一项...\n例如：\n项目1\n项目2\n项目3");
    m_fileDataEdit->setStyleSheet("QTextEdit { border: 1px solid #bdc3c7; border-radius: 3px; padding: 5px; }");
    fileLayout->addWidget(m_fileDataEdit);
    
    m_fileProgressBar = new QProgressBar(this);
    m_fileProgressBar->setStyleSheet("QProgressBar { border: 2px solid #bdc3c7; border-radius: 5px; text-align: center; } QProgressBar::chunk { background-color: #e67e22; border-radius: 3px; }");
    fileLayout->addWidget(m_fileProgressBar);
    
    m_startFileBtn = new QPushButton("开始写入文件", this);
    m_startFileBtn->setStyleSheet("QPushButton { background-color: #e67e22; color: white; padding: 8px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #d35400; } QPushButton:disabled { background-color: #95a5a6; }");
    fileLayout->addWidget(m_startFileBtn);
    
    m_fileStatus = new QLabel("就绪", this);
    m_fileStatus->setStyleSheet("QLabel { color: #7f8c8d; font-style: italic; }");
    fileLayout->addWidget(m_fileStatus);
    
    groupsLayout->addWidget(m_fileGroup);
    
    // ===== 数据处理组 =====
    m_dataGroup = new QGroupBox("数据处理线程", this);
    m_dataGroup->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *dataLayout = new QVBoxLayout(m_dataGroup);
    
    QLabel *inputLabel = new QLabel("输入数据 (每行一项):", this);
    dataLayout->addWidget(inputLabel);
    
    m_inputDataEdit = new QTextEdit(this);
    m_inputDataEdit->setMaximumHeight(100);
    m_inputDataEdit->setPlaceholderText("输入待处理的数据...\n例如：\nhello world\nqt programming\nmulti threading");
    m_inputDataEdit->setStyleSheet("QTextEdit { border: 1px solid #bdc3c7; border-radius: 3px; padding: 5px; }");
    dataLayout->addWidget(m_inputDataEdit);
    
    m_dataProgressBar = new QProgressBar(this);
    m_dataProgressBar->setStyleSheet("QProgressBar { border: 2px solid #bdc3c7; border-radius: 5px; text-align: center; } QProgressBar::chunk { background-color: #9b59b6; border-radius: 3px; }");
    dataLayout->addWidget(m_dataProgressBar);
    
    m_startDataBtn = new QPushButton("开始数据处理", this);
    m_startDataBtn->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; padding: 8px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #8e44ad; } QPushButton:disabled { background-color: #95a5a6; }");
    dataLayout->addWidget(m_startDataBtn);
    
    QLabel *outputLabel = new QLabel("处理结果:", this);
    dataLayout->addWidget(outputLabel);
    
    m_outputDataEdit = new QTextEdit(this);
    m_outputDataEdit->setMaximumHeight(120);
    m_outputDataEdit->setReadOnly(true);
    m_outputDataEdit->setStyleSheet("QTextEdit { border: 1px solid #bdc3c7; border-radius: 3px; padding: 5px; background-color: #f8f9fa; }");
    dataLayout->addWidget(m_outputDataEdit);
    
    m_dataStatus = new QLabel("就绪", this);
    m_dataStatus->setStyleSheet("QLabel { color: #7f8c8d; font-style: italic; }");
    dataLayout->addWidget(m_dataStatus);
    
    groupsLayout->addWidget(m_dataGroup);
    
    mainLayout->addLayout(groupsLayout);
    
    // 设置默认数据
    m_fileDataEdit->setPlainText("任务项目 1\n任务项目 2\n任务项目 3\n任务项目 4\n任务项目 5\n重要数据记录\n系统配置信息\n用户偏好设置");
    m_inputDataEdit->setPlainText("hello world\nqt programming\nmulti threading\nuser interface\ndata processing\nfile operations\nthread management\nreal time updates");
    
    // 设置默认文件路径
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/qt_output.txt";
    m_filePathEdit->setText(defaultPath);
}

void MainWindow::setupConnections()
{
    // 进度线程连接
    connect(m_progressThread, &ProgressThread::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(m_progressThread, &ProgressThread::progressFinished, this, &MainWindow::onProgressFinished);
    connect(m_startProgressBtn, &QPushButton::clicked, this, &MainWindow::startProgressThread);
    connect(m_stopProgressBtn, &QPushButton::clicked, this, &MainWindow::stopProgressThread);
    
    // 文件写入线程连接
    connect(m_fileWriterThread, &FileWriterThread::fileWriteProgress, this, &MainWindow::onFileWriteProgress);
    connect(m_fileWriterThread, &FileWriterThread::fileWriteFinished, this, &MainWindow::onFileWriteFinished);
    connect(m_fileWriterThread, &FileWriterThread::fileWriteError, this, &MainWindow::onFileWriteError);
    connect(m_selectFileBtn, &QPushButton::clicked, this, &MainWindow::selectOutputFile);
    connect(m_startFileBtn, &QPushButton::clicked, this, &MainWindow::startFileWriteThread);
    
    // 数据处理线程连接
    connect(m_dataProcessThread, &DataProcessThread::dataProcessed, this, &MainWindow::onDataProcessed);
    connect(m_dataProcessThread, &DataProcessThread::processProgress, this, &MainWindow::onProcessProgress);
    connect(m_dataProcessThread, &DataProcessThread::processFinished, this, &MainWindow::onProcessFinished);
    connect(m_startDataBtn, &QPushButton::clicked, this, &MainWindow::startDataProcessThread);
}

void MainWindow::onProgressUpdated(int value)
{
    m_progressBar->setValue(value);
    m_progressStatus->setText(QString("进行中... (%1%)").arg(value));
}

void MainWindow::onProgressFinished()
{
    m_progressStatus->setText("完成!");
    m_startProgressBtn->setEnabled(true);
    m_stopProgressBtn->setEnabled(false);
}

void MainWindow::onFileWriteProgress(int percentage)
{
    m_fileProgressBar->setValue(percentage);
    m_fileStatus->setText(QString("写入中... (%1%)").arg(percentage));
}

void MainWindow::onFileWriteFinished(const QString &message)
{
    m_fileStatus->setText("写入完成!");
    m_startFileBtn->setEnabled(true);
    QMessageBox::information(this, "文件写入", message);
}

void MainWindow::onFileWriteError(const QString &error)
{
    m_fileStatus->setText("写入错误!");
    m_startFileBtn->setEnabled(true);
    QMessageBox::critical(this, "文件写入错误", error);
}

void MainWindow::onDataProcessed(const QString &result)
{
    m_outputDataEdit->setPlainText(result);
}

void MainWindow::onProcessProgress(int percentage)
{
    m_dataProgressBar->setValue(percentage);
    m_dataStatus->setText(QString("处理中... (%1%)").arg(percentage));
}

void MainWindow::onProcessFinished()
{
    m_dataStatus->setText("处理完成!");
    m_startDataBtn->setEnabled(true);
}

void MainWindow::startProgressThread()
{
    if (m_progressThread->isRunning()) {
        return;
    }
    
    int maxValue = m_progressMaxSpin->value();
    m_progressThread->setProgressRange(0, maxValue);
    m_progressBar->setRange(0, maxValue);
    m_progressBar->setValue(0);
    
    m_startProgressBtn->setEnabled(false);
    m_stopProgressBtn->setEnabled(true);
    m_progressStatus->setText("启动中...");
    
    m_progressThread->start();
}

void MainWindow::stopProgressThread()
{
    if (m_progressThread->isRunning()) {
        m_progressThread->stopProgress();
        m_progressStatus->setText("停止中...");
        m_startProgressBtn->setEnabled(true);
        m_stopProgressBtn->setEnabled(false);
    }
}

void MainWindow::startFileWriteThread()
{
    if (m_fileWriterThread->isRunning()) {
        return;
    }
    
    QString fileName = m_filePathEdit->text().trimmed();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择输出文件路径!");
        return;
    }
    
    QString dataText = m_fileDataEdit->toPlainText().trimmed();
    if (dataText.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要写入的数据!");
        return;
    }
    
    QStringList dataList = dataText.split('\n', Qt::SkipEmptyParts);
    if (dataList.isEmpty()) {
        QMessageBox::warning(this, "警告", "数据格式无效!");
        return;
    }
    
    m_fileWriterThread->setFileData(fileName, dataList);
    m_fileProgressBar->setValue(0);
    m_startFileBtn->setEnabled(false);
    m_fileStatus->setText("启动中...");
    
    m_fileWriterThread->start();
}

void MainWindow::startDataProcessThread()
{
    if (m_dataProcessThread->isRunning()) {
        return;
    }
    
    QString inputText = m_inputDataEdit->toPlainText().trimmed();
    if (inputText.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入待处理的数据!");
        return;
    }
    
    QStringList inputList = inputText.split('\n', Qt::SkipEmptyParts);
    if (inputList.isEmpty()) {
        QMessageBox::warning(this, "警告", "输入数据格式无效!");
        return;
    }
    
    m_dataProcessThread->setProcessData(inputList);
    m_dataProgressBar->setValue(0);
    m_outputDataEdit->clear();
    m_startDataBtn->setEnabled(false);
    m_dataStatus->setText("启动中...");
    
    m_dataProcessThread->start();
}

void MainWindow::selectOutputFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "选择输出文件", 
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/qt_output.txt",
        "文本文件 (*.txt);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        m_filePathEdit->setText(fileName);
    }
}

void MainWindow::updateThreadStatus()
{
    int runningThreads = 0;
    QStringList activeThreads;
    
    if (m_progressThread && m_progressThread->isRunning()) {
        runningThreads++;
        activeThreads << "进度更新";
    }
    
    if (m_fileWriterThread && m_fileWriterThread->isRunning()) {
        runningThreads++;
        activeThreads << "文件写入";
    }
    
    if (m_dataProcessThread && m_dataProcessThread->isRunning()) {
        runningThreads++;
        activeThreads << "数据处理";
    }
    
    QString statusText;
    if (runningThreads == 0) {
        statusText = "系统就绪 - 所有线程空闲";
        m_overallStatus->setStyleSheet("QLabel { font-size: 14px; color: #27ae60; padding: 5px; background-color: #d5f4e6; border-radius: 3px; }");
    } else {
        statusText = QString("运行中 - %1个活动线程: %2").arg(runningThreads).arg(activeThreads.join(", "));
        m_overallStatus->setStyleSheet("QLabel { font-size: 14px; color: #e67e22; padding: 5px; background-color: #fdf2e9; border-radius: 3px; }");
    }
    
    m_overallStatus->setText(statusText);
}

#include "mainwindow.moc"
