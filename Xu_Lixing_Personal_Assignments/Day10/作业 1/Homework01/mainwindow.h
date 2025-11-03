#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTimer>
#include <QMutex>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 进度更新线程
class ProgressThread : public QThread
{
    Q_OBJECT

public:
    explicit ProgressThread(QObject *parent = nullptr);
    void setProgressRange(int min, int max);
    void stopProgress();

protected:
    void run() override;

signals:
    void progressUpdated(int value);
    void progressFinished();

private:
    int m_min;
    int m_max;
    bool m_stopped;
    QMutex m_mutex;
};

// 文件写入线程
class FileWriterThread : public QThread
{
    Q_OBJECT

public:
    explicit FileWriterThread(QObject *parent = nullptr);
    void setFileData(const QString &fileName, const QStringList &data);

protected:
    void run() override;

signals:
    void fileWriteProgress(int percentage);
    void fileWriteFinished(const QString &message);
    void fileWriteError(const QString &error);

private:
    QString m_fileName;
    QStringList m_data;
    QMutex m_mutex;
};

// 数据处理线程
class DataProcessThread : public QThread
{
    Q_OBJECT

public:
    explicit DataProcessThread(QObject *parent = nullptr);
    void setProcessData(const QStringList &data);

protected:
    void run() override;

signals:
    void dataProcessed(const QString &result);
    void processProgress(int percentage);
    void processFinished();

private:
    QStringList m_inputData;
    QMutex m_mutex;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onProgressUpdated(int value);
    void onProgressFinished();
    void onFileWriteProgress(int percentage);
    void onFileWriteFinished(const QString &message);
    void onFileWriteError(const QString &error);
    void onDataProcessed(const QString &result);
    void onProcessProgress(int percentage);
    void onProcessFinished();
    
    void startProgressThread();
    void stopProgressThread();
    void startFileWriteThread();
    void startDataProcessThread();
    void selectOutputFile();

private:
    void setupUI();
    void setupConnections();
    void updateThreadStatus();

    Ui::MainWindow *ui;
    
    // 线程对象
    ProgressThread *m_progressThread;
    FileWriterThread *m_fileWriterThread;
    DataProcessThread *m_dataProcessThread;
    
    // UI 组件
    QWidget *m_centralWidget;
    
    // 进度线程组
    QGroupBox *m_progressGroup;
    QProgressBar *m_progressBar;
    QPushButton *m_startProgressBtn;
    QPushButton *m_stopProgressBtn;
    QSpinBox *m_progressMaxSpin;
    QLabel *m_progressStatus;
    
    // 文件写入组
    QGroupBox *m_fileGroup;
    QLineEdit *m_filePathEdit;
    QPushButton *m_selectFileBtn;
    QPushButton *m_startFileBtn;
    QTextEdit *m_fileDataEdit;
    QProgressBar *m_fileProgressBar;
    QLabel *m_fileStatus;
    
    // 数据处理组
    QGroupBox *m_dataGroup;
    QTextEdit *m_inputDataEdit;
    QPushButton *m_startDataBtn;
    QTextEdit *m_outputDataEdit;
    QProgressBar *m_dataProgressBar;
    QLabel *m_dataStatus;
    
    // 整体状态
    QLabel *m_overallStatus;
    
    QTimer *m_statusTimer;
};

#endif // MAINWINDOW_H
