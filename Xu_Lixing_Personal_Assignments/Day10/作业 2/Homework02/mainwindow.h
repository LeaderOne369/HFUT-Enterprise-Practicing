#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QQueue>
#include <QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QLCDNumber>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 产品数据结构
struct Product {
    int id;
    QString name;
    QDateTime timestamp;
    
    Product(int _id = 0, const QString& _name = "", const QDateTime& _timestamp = QDateTime::currentDateTime()) 
        : id(_id), name(_name), timestamp(_timestamp) {}
};

// 生产者线程
class ProducerThread : public QThread {
    Q_OBJECT
    
public:
    ProducerThread(QQueue<Product>* buffer, QMutex* mutex, QSemaphore* usedSpaces, QSemaphore* freeSpaces, int maxProducts = 50);
    void setRunning(bool running);
    void setProductionSpeed(int speed);
    
protected:
    void run() override;
    
signals:
    void productProduced(const Product& product);
    void bufferStatusChanged(int size);
    
private:
    QQueue<Product>* m_buffer;
    QMutex* m_mutex;
    QSemaphore* m_usedSpaces;
    QSemaphore* m_freeSpaces;
    bool m_running;
    int m_productionSpeed;
    int m_maxProducts;
    int m_productCounter;
};

// 消费者线程
class ConsumerThread : public QThread {
    Q_OBJECT
    
public:
    ConsumerThread(QQueue<Product>* buffer, QMutex* mutex, QSemaphore* usedSpaces, QSemaphore* freeSpaces);
    void setRunning(bool running);
    void setConsumptionSpeed(int speed);
    
protected:
    void run() override;
    
signals:
    void productConsumed(const Product& product);
    void bufferStatusChanged(int size);
    
private:
    QQueue<Product>* m_buffer;
    QMutex* m_mutex;
    QSemaphore* m_usedSpaces;
    QSemaphore* m_freeSpaces;
    bool m_running;
    int m_consumptionSpeed;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onProductProduced(const Product& product);
    void onProductConsumed(const Product& product);
    void onBufferStatusChanged(int size);
    void onProductionSpeedChanged(int speed);
    void onConsumptionSpeedChanged(int speed);
    void updateStatistics();

private:
    void setupUI();
    void initializeThreads();
    void updateBufferVisualization();
    
    Ui::MainWindow *ui;
    
    // 线程同步相关
    QQueue<Product> m_buffer;
    static const int BUFFER_SIZE = 10;
    QMutex m_mutex;
    QSemaphore m_usedSpaces;
    QSemaphore m_freeSpaces;
    
    // 线程
    ProducerThread* m_producer;
    ConsumerThread* m_consumer;
    
    // UI组件
    QPushButton* m_startBtn;
    QPushButton* m_stopBtn;
    QSpinBox* m_productionSpeedSpin;
    QSpinBox* m_consumptionSpeedSpin;
    QProgressBar* m_bufferProgress;
    QTextEdit* m_producerLog;
    QTextEdit* m_consumerLog;
    QLCDNumber* m_producedCountLCD;
    QLCDNumber* m_consumedCountLCD;
    QLabel* m_bufferSizeLabel;
    QLabel* m_bufferVisualization;
    
    // 统计信息
    int m_producedCount;
    int m_consumedCount;
    QTimer* m_statisticsTimer;
};

#endif // MAINWINDOW_H
