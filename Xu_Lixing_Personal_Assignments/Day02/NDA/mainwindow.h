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

private:
    void setupUI();
    void setupUdpTab();
    void setupTcpTabs();
    QString getLocalIP();
    
    Ui::MainWindow *ui;
    
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
};

#endif // MAINWINDOW_H
