#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>

class ChatClient : public QObject
{
    Q_OBJECT

public:
    explicit ChatClient(QObject *parent = nullptr);
    ~ChatClient();

    bool connectToServer(const QString &host, quint16 port, const QString &nickname);
    void disconnectFromServer();
    void sendMessage(const QString &message);
    void sendPrivateMessage(const QString &receiver, const QString &message);
    void changeNickname(const QString &newNickname);
    void requestFileTransfer(const QString &receiver, const QString &fileName, qint64 fileSize, quint16 transferPort);
    void respondToFileTransfer(const QString &sender, bool accept, quint16 transferPort = 0);
    
    bool isConnected() const;
    QString getCurrentNickname() const { return currentNickname; }
    QStringList getOnlineUsers() const { return onlineUsers; }

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &sender, const QString &message, const QDateTime &timestamp);
    void privateMessageReceived(const QString &sender, const QString &message, const QDateTime &timestamp);
    void systemMessageReceived(const QString &message, const QDateTime &timestamp);
    void userJoined(const QString &nickname);
    void userLeft(const QString &nickname);
    void nicknameChanged(const QString &oldNickname, const QString &newNickname);
    void onlineUsersUpdated(const QStringList &users);
    void fileTransferRequested(const QString &sender, const QString &fileName, qint64 fileSize);
    void fileTransferResponse(const QString &responder, bool accepted, quint16 transferPort);
    void connectionError(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onReadyRead();
    void onHeartbeatTimeout();

private:
    QTcpSocket* socket;
    QTimer* heartbeatTimer;
    QString currentNickname;
    QStringList onlineUsers;
    
    void processMessage(const QJsonObject &message);
    void sendJsonMessage(const QJsonObject &message);
    QDateTime parseTimestamp(const QString &timestampStr);
};

#endif // CHATCLIENT_H 