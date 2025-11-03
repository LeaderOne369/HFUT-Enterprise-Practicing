#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextCharFormat>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTextDocumentFragment>

#include "chatserver.h"
#include "chatclient.h"
#include "filetransfer.h"
#include "database.h"
#include "connectdialog.h"
#include "privatechatwindow.h"

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

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // 连接相关
    void showConnectDialog();
    void onConnected();
    void onDisconnected();
    void onConnectionError(const QString &error);
    
    // 消息相关
    void onMessageReceived(const QString &sender, const QString &message, const QDateTime &timestamp);
    void onPrivateMessageReceived(const QString &sender, const QString &message, const QDateTime &timestamp);
    void onSystemMessageReceived(const QString &message, const QDateTime &timestamp);
    void sendMessage();
    
    // 用户相关
    void onUserJoined(const QString &nickname);
    void onUserLeft(const QString &nickname);
    void onOnlineUsersUpdated(const QStringList &users);
    void onUserListDoubleClicked();
    void changeNickname();
    
    // 文件传输相关
    void sendFile();
    void onFileTransferRequested(const QString &sender, const QString &fileName, qint64 fileSize);
    void onFileTransferResponse(const QString &responder, bool accepted, quint16 transferPort);
    void onTransferStarted();
    void onTransferProgress(int percentage, qint64 bytesTransferred, qint64 totalBytes);
    void onTransferCompleted();
    void onTransferFailed(const QString &error);
    
    // 服务器相关
    void onClientConnected(const QString &clientInfo);
    void onClientDisconnected(const QString &clientInfo);
    void onServerMessageReceived(const QString &sender, const QString &message);
    
    // UI相关
    void changeFont();
    void changeTextColor();
    void showChatHistory();
    void clearChatHistory();
    void showAbout();

private:
    Ui::MainWindow *ui;
    
    // 核心组件
    ChatServer* server;
    ChatClient* client;
    FileTransfer* fileTransfer;
    Database* database;
    
    // UI组件
    QSplitter* mainSplitter;
    QTabWidget* rightTabWidget;
    QListWidget* userListWidget;
    QTextEdit* chatDisplay;
    QTextEdit* messageInput;
    QPushButton* sendButton;
    QPushButton* fileButton;
    QPushButton* fontButton;
    QPushButton* colorButton;
    QProgressBar* transferProgressBar;
    QLabel* statusLabel;
    
    // 状态变量
    bool isServerMode;
    QString currentNickname;
    QTextCharFormat currentMessageFormat;
    FileTransfer* currentFileReceiver;
    QString pendingFileTransferSender;
    QString pendingFileName;
    qint64 pendingFileSize;
    QMap<QString, PrivateChatWindow*> privateChatWindows;
    
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();
    void applyStyles();
    void addMessageToChat(const QString &sender, const QString &message, const QDateTime &timestamp, bool isPrivate = false, bool isSystem = false);
    void addFormattedMessageToChat(const QString &sender, const QString &formattedMessage, const QDateTime &timestamp, bool isPrivate = false, bool isSystem = false);
    void updateUserList(const QStringList &users);
    void updateUserListWithPrivateMessageIndicator(const QString &sender);
    void showFileTransferDialog(const QString &sender, const QString &fileName, qint64 fileSize);
    QString formatFileSize(qint64 bytes);
    QString formatTimestamp(const QDateTime &timestamp);
    void loadChatHistory();
    void saveMessageToDatabase(const QString &sender, const QString &message, bool isFile = false, const QString &filePath = "");
};

#endif // MAINWINDOW_H
