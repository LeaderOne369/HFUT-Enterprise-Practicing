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

private:
    void setupUI();
    void setupUdpTab();
    void setupTcpTabs();
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
    
    // 主界面组件
    QTabWidget *tabWidget;
    QWidget *udpTab;
    QWidget *tcpServerTab;
    QWidget *tcpClientTab;
    
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
    
    QString localIP;
    bool isPortBound;
    
    // 统计数据
    int messageCount;
    qint64 totalBytesReceived;
    qint64 totalBytesSent;
    bool isPaused;
};

#endif // MAINWINDOW_H
