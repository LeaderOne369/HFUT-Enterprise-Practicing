#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTimer>

struct ClientInfo {
    QTcpSocket* socket;
    QString nickname;
    QString address;
    quint16 port;
};

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit ChatServer(QObject *parent = nullptr);
    ~ChatServer();
    
    bool startServer(quint16 port = 8888);
    void stopServer();
    QStringList getConnectedClients();
    void sendMessageToAll(const QString &message, const QString &sender = "Server");
    void sendMessageToClient(const QString &clientId, const QString &message, const QString &sender = "Server");

signals:
    void clientConnected(const QString &clientInfo);
    void clientDisconnected(const QString &clientInfo);
    void messageReceived(const QString &sender, const QString &message);
    void fileTransferRequested(const QString &sender, const QString &receiver, const QString &fileName, qint64 fileSize);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected();
    void onMessageReceived();
    void onHeartbeatTimeout();

private:
    QList<ClientInfo> clients;
    QTimer* heartbeatTimer;
    
    void broadcastMessage(const QJsonObject &message, QTcpSocket* excludeSocket = nullptr);
    void sendToClient(QTcpSocket* socket, const QJsonObject &message);
    ClientInfo* findClientBySocket(QTcpSocket* socket);
    ClientInfo* findClientByNickname(const QString &nickname);
    void removeClient(QTcpSocket* socket);
    QJsonObject createMessage(const QString &type, const QString &content, const QString &sender = "");
};

#endif // CHATSERVER_H 