#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QFont>
#include <QStyle>

// 生产者线程实现
ProducerThread::ProducerThread(QQueue<Product>* buffer, QMutex* mutex, QSemaphore* usedSpaces, QSemaphore* freeSpaces, int maxProducts)
    : m_buffer(buffer), m_mutex(mutex), m_usedSpaces(usedSpaces), m_freeSpaces(freeSpaces), 
      m_running(false), m_productionSpeed(1000), m_maxProducts(maxProducts), m_productCounter(0)
{
}

void ProducerThread::setRunning(bool running) {
    m_running = running;
}

void ProducerThread::setProductionSpeed(int speed) {
    m_productionSpeed = speed;
}

void ProducerThread::run() {
    QStringList productNames = {"智能手机", "笔记本电脑", "平板电脑", "智能手表", "耳机", "相机", "游戏机", "电视", "音响", "路由器"};
    
    while (m_running && m_productCounter < m_maxProducts) {
        // 等待缓冲区有空闲空间
        m_freeSpaces->acquire();
        
        if (!m_running) break;
        
        // 创建产品
        Product product(++m_productCounter, 
                       productNames[m_productCounter % productNames.size()],
                       QDateTime::currentDateTime());
        
        // 加锁访问共享缓冲区
        m_mutex->lock();
        m_buffer->enqueue(product);
        int bufferSize = m_buffer->size();
        m_mutex->unlock();
        
        // 释放一个已使用空间信号量
        m_usedSpaces->release();
        
        emit productProduced(product);
        emit bufferStatusChanged(bufferSize);
        
        // 生产速度控制
        msleep(m_productionSpeed);
    }
}

// 消费者线程实现
ConsumerThread::ConsumerThread(QQueue<Product>* buffer, QMutex* mutex, QSemaphore* usedSpaces, QSemaphore* freeSpaces)
    : m_buffer(buffer), m_mutex(mutex), m_usedSpaces(usedSpaces), m_freeSpaces(freeSpaces), 
      m_running(false), m_consumptionSpeed(1500)
{
}

void ConsumerThread::setRunning(bool running) {
    m_running = running;
}

void ConsumerThread::setConsumptionSpeed(int speed) {
    m_consumptionSpeed = speed;
}

void ConsumerThread::run() {
    while (m_running) {
        // 等待缓冲区有产品可消费
        m_usedSpaces->acquire();
        
        if (!m_running) break;
        
        // 加锁访问共享缓冲区
        m_mutex->lock();
        Product product = m_buffer->dequeue();
        int bufferSize = m_buffer->size();
        m_mutex->unlock();
        
        // 释放一个空闲空间信号量
        m_freeSpaces->release();
        
        emit productConsumed(product);
        emit bufferStatusChanged(bufferSize);
        
        // 消费速度控制
        msleep(m_consumptionSpeed);
    }
}

// 主窗口实现
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_usedSpaces(0)
    , m_freeSpaces(BUFFER_SIZE)
    , m_producer(nullptr)
    , m_consumer(nullptr)
    , m_producedCount(0)
    , m_consumedCount(0)
{
    ui->setupUi(this);
    setupUI();
    initializeThreads();
    
    // 设置定时器更新统计信息
    m_statisticsTimer = new QTimer(this);
    connect(m_statisticsTimer, &QTimer::timeout, this, &MainWindow::updateStatistics);
    m_statisticsTimer->start(100); // 100ms更新一次
}

MainWindow::~MainWindow()
{
    if (m_producer && m_producer->isRunning()) {
        m_producer->setRunning(false);
        m_producer->quit();
        m_producer->wait();
    }
    if (m_consumer && m_consumer->isRunning()) {
        m_consumer->setRunning(false);
        m_consumer->quit();
        m_consumer->wait();
    }
    delete ui;
}

void MainWindow::setupUI() {
    setWindowTitle("生产者-消费者线程同步演示系统");
    setMinimumSize(1200, 800);
    
    // 设置应用样式
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QGroupBox {
            font: bold 12px;
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
            color: #2196F3;
        }
        QPushButton {
            background-color: #2196F3;
            border: none;
            color: white;
            padding: 8px 16px;
            border-radius: 4px;
            font: bold 12px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:pressed {
            background-color: #0D47A1;
        }
        QPushButton:disabled {
            background-color: #cccccc;
        }
        QTextEdit {
            border: 1px solid #ddd;
            border-radius: 4px;
            font: 10px "Consolas", "Monaco", monospace;
            background-color: #fafafa;
        }
        QLCDNumber {
            background-color: #000000;
            color: #00FF00;
            border: 2px solid #333333;
            border-radius: 8px;
        }
        QProgressBar {
            border: 2px solid #cccccc;
            border-radius: 8px;
            text-align: center;
            font: bold 12px;
        }
        QProgressBar::chunk {
            background-color: #4CAF50;
            border-radius: 6px;
        }
        QSpinBox {
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 4px;
            font: 12px;
        }
    )");
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 标题
    QLabel* titleLabel = new QLabel("生产者-消费者线程同步演示系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font: bold 20px; color: #2196F3; margin: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // 控制面板
    QGroupBox* controlGroup = new QGroupBox("控制面板", this);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlGroup);
    
    m_startBtn = new QPushButton("开始生产", this);
    m_stopBtn = new QPushButton("停止生产", this);
    m_stopBtn->setEnabled(false);
    m_stopBtn->setStyleSheet("QPushButton { background-color: #f44336; } QPushButton:hover { background-color: #d32f2f; }");
    
    controlLayout->addWidget(new QLabel("生产速度(ms):"));
    m_productionSpeedSpin = new QSpinBox(this);
    m_productionSpeedSpin->setRange(100, 5000);
    m_productionSpeedSpin->setValue(1000);
    m_productionSpeedSpin->setSuffix(" ms");
    controlLayout->addWidget(m_productionSpeedSpin);
    
    controlLayout->addWidget(new QLabel("消费速度(ms):"));
    m_consumptionSpeedSpin = new QSpinBox(this);
    m_consumptionSpeedSpin->setRange(100, 5000);
    m_consumptionSpeedSpin->setValue(1500);
    m_consumptionSpeedSpin->setSuffix(" ms");
    controlLayout->addWidget(m_consumptionSpeedSpin);
    
    controlLayout->addStretch();
    controlLayout->addWidget(m_startBtn);
    controlLayout->addWidget(m_stopBtn);
    
    mainLayout->addWidget(controlGroup);
    
    // 状态面板
    QGroupBox* statusGroup = new QGroupBox("系统状态", this);
    QGridLayout* statusLayout = new QGridLayout(statusGroup);
    
    statusLayout->addWidget(new QLabel("已生产:"), 0, 0);
    m_producedCountLCD = new QLCDNumber(4, this);
    m_producedCountLCD->setFixedHeight(60);
    statusLayout->addWidget(m_producedCountLCD, 0, 1);
    
    statusLayout->addWidget(new QLabel("已消费:"), 0, 2);
    m_consumedCountLCD = new QLCDNumber(4, this);
    m_consumedCountLCD->setFixedHeight(60);
    statusLayout->addWidget(m_consumedCountLCD, 0, 3);
    
    statusLayout->addWidget(new QLabel("缓冲区状态:"), 1, 0);
    m_bufferProgress = new QProgressBar(this);
    m_bufferProgress->setRange(0, BUFFER_SIZE);
    m_bufferProgress->setValue(0);
    m_bufferProgress->setFormat("缓冲区: %v/%m");
    statusLayout->addWidget(m_bufferProgress, 1, 1, 1, 3);
    
    // 缓冲区可视化
    m_bufferVisualization = new QLabel(this);
    m_bufferVisualization->setFixedHeight(40);
    m_bufferVisualization->setStyleSheet("border: 1px solid #ddd; background-color: white; font: 12px;");
    m_bufferVisualization->setAlignment(Qt::AlignCenter);
    updateBufferVisualization();
    statusLayout->addWidget(new QLabel("缓冲区可视化:"), 2, 0);
    statusLayout->addWidget(m_bufferVisualization, 2, 1, 1, 3);
    
    mainLayout->addWidget(statusGroup);
    
    // 日志面板
    QHBoxLayout* logLayout = new QHBoxLayout();
    
    QGroupBox* producerGroup = new QGroupBox("生产者日志", this);
    QVBoxLayout* producerLayout = new QVBoxLayout(producerGroup);
    m_producerLog = new QTextEdit(this);
    producerLayout->addWidget(m_producerLog);
    
    QGroupBox* consumerGroup = new QGroupBox("消费者日志", this);
    QVBoxLayout* consumerLayout = new QVBoxLayout(consumerGroup);
    m_consumerLog = new QTextEdit(this);
    consumerLayout->addWidget(m_consumerLog);
    
    logLayout->addWidget(producerGroup);
    logLayout->addWidget(consumerGroup);
    
    mainLayout->addLayout(logLayout);
    
    // 连接信号槽
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_productionSpeedSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onProductionSpeedChanged);
    connect(m_consumptionSpeedSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onConsumptionSpeedChanged);
}

void MainWindow::initializeThreads() {
    m_producer = new ProducerThread(&m_buffer, &m_mutex, &m_usedSpaces, &m_freeSpaces, 50);
    m_consumer = new ConsumerThread(&m_buffer, &m_mutex, &m_usedSpaces, &m_freeSpaces);
    
    connect(m_producer, &ProducerThread::productProduced, this, &MainWindow::onProductProduced);
    connect(m_producer, &ProducerThread::bufferStatusChanged, this, &MainWindow::onBufferStatusChanged);
    connect(m_consumer, &ConsumerThread::productConsumed, this, &MainWindow::onProductConsumed);
    connect(m_consumer, &ConsumerThread::bufferStatusChanged, this, &MainWindow::onBufferStatusChanged);
}

void MainWindow::onStartClicked() {
    m_producedCount = 0;
    m_consumedCount = 0;
    m_producerLog->clear();
    m_consumerLog->clear();
    
    m_producer->setRunning(true);
    m_consumer->setRunning(true);
    m_producer->start();
    m_consumer->start();
    
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_productionSpeedSpin->setEnabled(false);
    m_consumptionSpeedSpin->setEnabled(false);
}

void MainWindow::onStopClicked() {
    m_producer->setRunning(false);
    m_consumer->setRunning(false);
    
    // 释放可能等待的信号量
    m_freeSpaces.release(BUFFER_SIZE);
    m_usedSpaces.release(BUFFER_SIZE);
    
    m_producer->quit();
    m_consumer->quit();
    m_producer->wait();
    m_consumer->wait();
    
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_productionSpeedSpin->setEnabled(true);
    m_consumptionSpeedSpin->setEnabled(true);
}

void MainWindow::onProductProduced(const Product& product) {
    QString logText = QString("[%1] 生产产品 #%2: %3")
                          .arg(product.timestamp.toString("hh:mm:ss.zzz"))
                          .arg(product.id)
                          .arg(product.name);
    m_producerLog->append(logText);
    m_producedCount++;
}

void MainWindow::onProductConsumed(const Product& product) {
    QString logText = QString("[%1] 消费产品 #%2: %3")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                          .arg(product.id)
                          .arg(product.name);
    m_consumerLog->append(logText);
    m_consumedCount++;
}

void MainWindow::onBufferStatusChanged(int size) {
    m_bufferProgress->setValue(size);
    updateBufferVisualization();
}

void MainWindow::onProductionSpeedChanged(int speed) {
    if (m_producer) {
        m_producer->setProductionSpeed(speed);
    }
}

void MainWindow::onConsumptionSpeedChanged(int speed) {
    if (m_consumer) {
        m_consumer->setConsumptionSpeed(speed);
    }
}

void MainWindow::updateStatistics() {
    m_producedCountLCD->display(m_producedCount);
    m_consumedCountLCD->display(m_consumedCount);
}

void MainWindow::updateBufferVisualization() {
    QString visualization = "[ ";
    m_mutex.lock();
    int bufferSize = m_buffer.size();
    m_mutex.unlock();
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (i < bufferSize) {
            visualization += "■ ";
        } else {
            visualization += "□ ";
        }
    }
    visualization += "]";
    
    m_bufferVisualization->setText(visualization);
    
    // 根据缓冲区状态改变颜色
    if (bufferSize == 0) {
        m_bufferVisualization->setStyleSheet("border: 1px solid #ddd; background-color: #ffebee; color: #d32f2f; font: 12px;");
    } else if (bufferSize == BUFFER_SIZE) {
        m_bufferVisualization->setStyleSheet("border: 1px solid #ddd; background-color: #fff3e0; color: #f57c00; font: 12px;");
    } else {
        m_bufferVisualization->setStyleSheet("border: 1px solid #ddd; background-color: #e8f5e8; color: #388e3c; font: 12px;");
    }
}

#include "mainwindow.moc"
