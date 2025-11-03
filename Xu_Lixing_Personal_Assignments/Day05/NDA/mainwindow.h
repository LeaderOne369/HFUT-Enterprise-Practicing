#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QTimer>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void bindPort();
    void closePort();
    void bindGroup();
    void sendData();
    void broadcastSend();
    void sendHex();
    void clearSendArea();
    void clearReceiveArea();
    void onTimerSend();
    void onUdpReceiveData();
    
    // 菜单栏功能槽函数
    void newSession();
    void openSession();
    void saveSession();
    void exitApplication();
    void showAbout();
    void showHelp();
    void showSettings();
    void connectToHost();
    void disconnectFromHost();
    void pauseResume();
    
    // 新增功能槽函数
    void updateDateTime();
    void startPing();
    void onPingFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onPingError(QProcess::ProcessError error);
    void updateLocalIP();
    void startPortScan();
    void onPortScanFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onPortScanError(QProcess::ProcessError error);
    void onTimerPing();

private:
    void setupUI();
    void setupUdpTab();
    void setupTcpTabs();
    void setupPingTab();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void createActions();
    void updateBackgroundImage();
    QString getLocalIP();

protected:
    void paintEvent(QPaintEvent *event) override;
    
    Ui::MainWindow *ui;
    
    // 菜单栏和工具栏
    QToolBar *mainToolBar;
    
    // 动作组件
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *exitAction;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *pauseAction;
    QAction *clearSendAction;
    QAction *clearReceiveAction;
    QAction *settingsAction;
    QAction *helpAction;
    QAction *aboutAction;
    
    // 状态栏组件
    QLabel *connectionStatusLabel;
    QLabel *messageCountLabel;
    QLabel *bytesCountLabel;
    QProgressBar *progressBar;
    QLabel *dateTimeLabel;  // 新增：显示当前时间的标签
    QLabel *pingStatusLabel;  // 新增：显示ping状态的标签
    
    // Ping测试相关组件
    QLineEdit *pingAddressEdit;  // 新增：ping地址输入框
    QPushButton *pingButton;     // 新增：ping按钮
    QTextEdit *pingResultEdit;   // 新增：ping结果显示区域
    QProcess *pingProcess;       // 新增：ping进程
    
    // 主界面组件
    QTabWidget *tabWidget;
    QWidget *udpTab;
    QWidget *tcpServerTab;
    QWidget *tcpClientTab;
    QWidget *pingTab;  // 新增：ping测试选项卡
    
    // 左侧参数设置区域
    QLabel *localIpLabel;
    QLabel *localIpValue;
    QLabel *localPortLabel;
    QLineEdit *localPortEdit;
    QPushButton *bindPortBtn;
    QPushButton *closePortBtn;
    QLabel *statusLabel;
    QLabel *targetIpLabel;
    QLineEdit *targetIpEdit;
    QLabel *targetPortLabel;
    QLineEdit *targetPortEdit;
    QLineEdit *groupIpEdit;
    QPushButton *bindGroupBtn;
    QTextEdit *hintTextEdit;
    
    // 右上发送区域
    QCheckBox *timerSendCheck;
    QLineEdit *intervalEdit;
    QLabel *broadcastIpLabel;
    QPushButton *sendBtn;
    QPushButton *broadcastSendBtn;
    QPushButton *sendHexBtn;
    QPushButton *clearSendBtn;
    QTextEdit *sendTextEdit;
    
    // 右下接收区域
    QCheckBox *hexDisplayCheck;
    QCheckBox *showIpCheck;
    QCheckBox *showTimeCheck;
    QCheckBox *showPortCheck;
    QPushButton *clearReceiveBtn;
    QTextEdit *receiveTextEdit;
    
    // 网络组件
    QUdpSocket *udpSocket;
    QTimer *sendTimer;
    QTimer *dateTimeTimer;  // 新增：用于更新时间显示的定时器
    QTimer *ipUpdateTimer;  // 新增：用于定时更新本地IP的定时器
    QTimer *pingTimer;      // 新增：用于定时ping的定时器
    
    QString localIP;
    bool isPortBound;
    bool isPingRunning;  // 新增：标记ping是否正在运行
    bool isPortScanRunning; // 新增：标记端口扫描是否正在运行
    
    // 统计数据
    int messageCount;
    qint64 totalBytesReceived;
    qint64 totalBytesSent;
    bool isPaused;
    
    // 新增ping相关控件
    QLineEdit *pingPortEdit;     // 新增：ping端口输入框
    QCheckBox *pingTimerCheck;   // 新增：定时ping复选框
    QLineEdit *pingIntervalEdit; // 新增：ping间隔输入框
    QPushButton *portScanButton; // 新增：端口扫描按钮
    QProcess *portScanProcess;   // 新增：端口扫描进程
};

#endif // MAINWINDOW_H
