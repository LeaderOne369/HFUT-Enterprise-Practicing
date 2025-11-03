#include "chatclient.h"
#include <QDateTime>

ChatClient::ChatClient(QObject *parent)
    : QObject(parent), socket(nullptr)
{
    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &ChatClient::onHeartbeatTimeout);
}

ChatClient::~ChatClient()
{
    disconnectFromServer();
}

bool ChatClient::connectToServer(const QString &host, quint16 port, const QString &nickname)
{
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        return false; // 已经连接
    }

    currentNickname = nickname;
    
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &ChatClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ChatClient::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &ChatClient::onErrorOccurred);
    connect(socket, &QTcpSocket::readyRead, this, &ChatClient::onReadyRead);

    socket->connectToHost(host, port);
    return true;
}

void ChatClient::disconnectFromServer()
{
    heartbeatTimer->stop();
    
    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
        socket = nullptr;
    }
    
    onlineUsers.clear();
    currentNickname.clear();
}

void ChatClient::sendMessage(const QString &message)
{
    if (!isConnected()) return;
    
    QJsonObject msg;
    msg["type"] = "message";
    msg["content"] = message;
    msg["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendJsonMessage(msg);
}

void ChatClient::sendPrivateMessage(const QString &receiver, const QString &message)
{
    if (!isConnected()) return;
    
    QJsonObject msg;
    msg["type"] = "private_message";
    msg["content"] = message;
    msg["receiver"] = receiver;
    msg["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendJsonMessage(msg);
}

void ChatClient::changeNickname(const QString &newNickname)
{
    if (!isConnected()) return;
    
    QJsonObject msg;
    msg["type"] = "nickname_change";
    msg["content"] = newNickname;
    
    sendJsonMessage(msg);
}

void ChatClient::requestFileTransfer(const QString &receiver, const QString &fileName, qint64 fileSize, quint16 transferPort)
{
    if (!isConnected()) return;
    
    QJsonObject msg;
    msg["type"] = "file_transfer_request";
    msg["receiver"] = receiver;
    msg["filename"] = fileName;
    msg["filesize"] = fileSize;
    msg["transfer_port"] = transferPort;
    
    sendJsonMessage(msg);
}

void ChatClient::respondToFileTransfer(const QString &sender, bool accept, quint16 transferPort)
{
    if (!isConnected()) return;
    
    QJsonObject msg;
    msg["type"] = "file_transfer_response";
    msg["sender"] = sender;
    msg["accepted"] = accept;
    if (accept && transferPort > 0) {
        msg["transfer_port"] = transferPort;
    }
    
    sendJsonMessage(msg);
}

bool ChatClient::isConnected() const
{
    return socket && socket->state() == QTcpSocket::ConnectedState;
}

void ChatClient::onConnected()
{
    qDebug() << "已连接到服务器";
    heartbeatTimer->start(35000); // 35秒发送一次心跳响应
    
    // 发送昵称注册
    QJsonObject msg;
    msg["type"] = "register_nickname";
    msg["content"] = currentNickname;
    sendJsonMessage(msg);
    
    emit connected();
}

void ChatClient::onDisconnected()
{
    qDebug() << "与服务器断开连接";
    heartbeatTimer->stop();
    onlineUsers.clear();
    emit disconnected();
}

void ChatClient::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QString errorMsg = socket ? socket->errorString() : "网络错误";
    qDebug() << "连接错误:" << errorMsg;
    emit connectionError(errorMsg);
}

void ChatClient::onReadyRead()
{
    if (!socket) return;
    
    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (doc.isObject()) {
            processMessage(doc.object());
        }
    }
}

void ChatClient::onHeartbeatTimeout()
{
    // 响应服务器心跳
    if (isConnected()) {
        QJsonObject msg;
        msg["type"] = "heartbeat_response";
        sendJsonMessage(msg);
    }
}

void ChatClient::processMessage(const QJsonObject &message)
{
    QString type = message["type"].toString();
    QString content = message["content"].toString();
    QString sender = message["sender"].toString();
    QDateTime timestamp = parseTimestamp(message["timestamp"].toString());
    
    if (type == "connection_established") {
        // 连接建立，发送昵称注册已在onConnected中处理
        
    } else if (type == "welcome") {
        currentNickname = message["nickname"].toString();
        emit systemMessageReceived(content, timestamp);
        
    } else if (type == "message") {
        emit messageReceived(sender, content, timestamp);
        
    } else if (type == "private_message") {
        emit privateMessageReceived(sender, content, timestamp);
        
    } else if (type == "user_joined") {
        emit systemMessageReceived(content, timestamp);
        emit userJoined(sender);
        
    } else if (type == "user_left") {
        emit systemMessageReceived(content, timestamp);
        emit userLeft(sender);
        
    } else if (type == "nickname_changed") {
        emit systemMessageReceived(content, timestamp);
        // 解析昵称变更信息
        // TODO: 可以进一步解析oldNickname和newNickname
        
    } else if (type == "user_list") {
        QJsonArray userArray = message["users"].toArray();
        onlineUsers.clear();
        for (const auto &user : userArray) {
            onlineUsers.append(user.toString());
        }
        emit onlineUsersUpdated(onlineUsers);
        
    } else if (type == "file_transfer_request") {
        QString fileName = message["filename"].toString();
        qint64 fileSize = message["filesize"].toInteger();
        emit fileTransferRequested(sender, fileName, fileSize);
        
    } else if (type == "file_transfer_response") {
        QString responder = message["responder"].toString();
        bool accepted = message["accepted"].toBool();
        quint16 transferPort = message["transfer_port"].toInt();
        emit fileTransferResponse(responder, accepted, transferPort);
        
    } else if (type == "server_shutdown") {
        emit systemMessageReceived(content, timestamp);
        disconnectFromServer();
        
    } else if (type == "heartbeat") {
        // 收到服务器心跳，发送响应
        QJsonObject response;
        response["type"] = "heartbeat_response";
        sendJsonMessage(response);
    }
}

void ChatClient::sendJsonMessage(const QJsonObject &message)
{
    if (!socket || socket->state() != QTcpSocket::ConnectedState) return;
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    socket->write(data);
}

QDateTime ChatClient::parseTimestamp(const QString &timestampStr)
{
    QDateTime timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    return timestamp.isValid() ? timestamp : QDateTime::currentDateTime();
} 