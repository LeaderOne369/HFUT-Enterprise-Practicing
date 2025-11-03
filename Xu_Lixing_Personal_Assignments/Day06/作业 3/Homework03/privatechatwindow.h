#ifndef PRIVATECHATWINDOW_H
#define PRIVATECHATWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QCloseEvent>
#include <QRandomGenerator>

class ChatClient;

class PrivateChatWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PrivateChatWindow(const QString &targetUser, const QString &currentUser, 
                              ChatClient *client, QWidget *parent = nullptr);
    ~PrivateChatWindow();

    void addMessage(const QString &sender, const QString &message, const QDateTime &timestamp);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void sendMessage();
    void insertEmoji();
    void sendFile();

private:
    void setupUI();
    void applyIOSStyle();
    void loadChatHistory();
    void saveChatHistory();
    void saveMessage(const QString &sender, const QString &message, const QDateTime &timestamp);
    void showNotification(const QString &message);
    QString formatFileSize(qint64 bytes);
    
    QString targetUser;
    QString currentUser;
    ChatClient *chatClient;
    
    QTextEdit *chatDisplay;
    QTextEdit *messageInput;
    QPushButton *sendButton;
    QLabel *titleLabel;
    QLabel *statusLabel;
};

#endif // PRIVATECHATWINDOW_H 