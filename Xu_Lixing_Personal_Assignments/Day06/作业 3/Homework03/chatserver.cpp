#include "chatserver.h"

ChatServer::ChatServer(QObject *parent)
    : QTcpServer(parent)
{
    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &ChatServer::onHeartbeatTimeout);
}

ChatServer::~ChatServer()
{
    stopServer();
}

bool ChatServer::startServer(quint16 port)
{
    if (listen(QHostAddress::Any, port)) {
        qDebug() << "服务器启动成功，端口:" << port;
        heartbeatTimer->start(30000); // 30秒心跳检测
        return true;
    } else {
        qDebug() << "服务器启动失败:" << errorString();
        return false;
    }
}

void ChatServer::stopServer()
{
    heartbeatTimer->stop();
    
    // 通知所有客户端服务器关闭
    QJsonObject message = createMessage("server_shutdown", "服务器即将关闭");
    broadcastMessage(message);
    
    // 关闭所有连接
    for (auto &client : clients) {
        client.socket->disconnectFromHost();
    }
    clients.clear();
    
    close();
    qDebug() << "服务器已关闭";
}

QStringList ChatServer::getConnectedClients()
{
    QStringList clientList;
    for (const auto &client : clients) {
        clientList << QString("%1 (%2:%3)").arg(client.nickname).arg(client.address).arg(client.port);
    }
    return clientList;
}

void ChatServer::sendMessageToAll(const QString &message, const QString &sender)
{
    QJsonObject msg = createMessage("message", message, sender);
    broadcastMessage(msg);
}

void ChatServer::sendMessageToClient(const QString &clientId, const QString &message, const QString &sender)
{
    ClientInfo* client = findClientByNickname(clientId);
    if (client) {
        QJsonObject msg = createMessage("private_message", message, sender);
        sendToClient(client->socket, msg);
    }
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    
    connect(socket, &QTcpSocket::disconnected, this, &ChatServer::onClientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &ChatServer::onMessageReceived);
    
    ClientInfo client;
    client.socket = socket;
    client.address = socket->peerAddress().toString();
    client.port = socket->peerPort();
    client.nickname = ""; // 初始为空，等待客户端发送昵称
    
    clients.append(client);
    
    // 发送连接确认，但不发送欢迎消息，等待昵称
    QJsonObject connectMsg = createMessage("connection_established", "", "Server");
    sendToClient(socket, connectMsg);
}

void ChatServer::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    ClientInfo* client = findClientBySocket(socket);
    if (client) {
        QString clientInfo = QString("%1 (%2:%3)").arg(client->nickname).arg(client->address).arg(client->port);
        
        // 通知其他用户有用户离开
        QJsonObject leaveMsg = createMessage("user_left", QString("%1 离开了聊天室").arg(client->nickname), "Server");
        broadcastMessage(leaveMsg, socket);
        
        emit clientDisconnected(clientInfo);
        removeClient(socket);
        
        // 广播更新的用户列表
        QJsonObject userListMsg = createMessage("user_list", "");
        QJsonArray userArray;
        for (const auto &c : clients) {
            if (!c.nickname.isEmpty()) {
                userArray.append(c.nickname);
            }
        }
        userListMsg["users"] = userArray;
        broadcastMessage(userListMsg);
    }
    
    socket->deleteLater();
}

void ChatServer::onMessageReceived()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    ClientInfo* client = findClientBySocket(socket);
    if (!client) return;
    
    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (!doc.isObject()) continue;
        
        QJsonObject message = doc.object();
        QString type = message["type"].toString();
        QString content = message["content"].toString();
        
        if (type == "register_nickname") {
            // 客户端注册昵称
            if (client->nickname.isEmpty()) {
                client->nickname = content;
                
                // 发送欢迎消息
                QJsonObject welcomeMsg = createMessage("welcome", "欢迎加入聊天室！", "Server");
                welcomeMsg["nickname"] = client->nickname;
                sendToClient(socket, welcomeMsg);
                
                // 发送在线用户列表给新用户
                QJsonObject userListMsg = createMessage("user_list", "");
                QJsonArray userArray;
                for (const auto &c : clients) {
                    if (!c.nickname.isEmpty()) {
                        userArray.append(c.nickname);
                    }
                }
                userListMsg["users"] = userArray;
                sendToClient(socket, userListMsg);
                
                // 广播用户列表更新给所有用户
                broadcastMessage(userListMsg);
                
                // 通知其他用户有新用户加入
                QJsonObject joinMsg = createMessage("user_joined", QString("%1 加入了聊天室").arg(client->nickname), "Server");
                broadcastMessage(joinMsg, socket);
                
                emit clientConnected(QString("%1 (%2:%3)").arg(client->nickname).arg(client->address).arg(client->port));
            }
            
        } else if (type == "message") {
            // 普通聊天消息
            if (!client->nickname.isEmpty()) {
                QJsonObject msg = createMessage("message", content, client->nickname);
                broadcastMessage(msg, socket);
                emit messageReceived(client->nickname, content);
            }
            
        } else if (type == "private_message") {
            // 私聊消息
            if (!client->nickname.isEmpty()) {
                QString receiver = message["receiver"].toString();
                ClientInfo* receiverClient = findClientByNickname(receiver);
                if (receiverClient) {
                    QJsonObject msg = createMessage("private_message", content, client->nickname);
                    sendToClient(receiverClient->socket, msg);
                    qDebug() << "转发私聊消息:" << client->nickname << "=>" << receiver << ":" << content;
                } else {
                    qDebug() << "私聊接收者不存在:" << receiver;
                }
            }
            
        } else if (type == "nickname_change") {
            // 修改昵称
            QString oldNickname = client->nickname;
            client->nickname = content;
            
            QJsonObject msg = createMessage("nickname_changed", 
                QString("%1 将昵称改为 %2").arg(oldNickname).arg(content), "Server");
            broadcastMessage(msg);
            
            // 广播更新的用户列表
            QJsonObject userListMsg = createMessage("user_list", "");
            QJsonArray userArray;
            for (const auto &c : clients) {
                if (!c.nickname.isEmpty()) {
                    userArray.append(c.nickname);
                }
            }
            userListMsg["users"] = userArray;
            broadcastMessage(userListMsg);
            
        } else if (type == "file_transfer_request") {
            // 文件传输请求
            QString receiver = message["receiver"].toString();
            QString fileName = message["filename"].toString();
            qint64 fileSize = message["filesize"].toInteger();
            
            ClientInfo* receiverClient = findClientByNickname(receiver);
            if (receiverClient) {
                QJsonObject fileMsg = message;
                fileMsg["sender"] = client->nickname;
                sendToClient(receiverClient->socket, fileMsg);
                emit fileTransferRequested(client->nickname, receiver, fileName, fileSize);
            }
            
        } else if (type == "file_transfer_response") {
            // 文件传输响应
            QString sender = message["sender"].toString();
            ClientInfo* senderClient = findClientByNickname(sender);
            if (senderClient) {
                QJsonObject responseMsg = message;
                responseMsg["responder"] = client->nickname;
                sendToClient(senderClient->socket, responseMsg);
            }
        }
    }
}

void ChatServer::onHeartbeatTimeout()
{
    // 发送心跳包给所有客户端
    QJsonObject heartbeat = createMessage("heartbeat", "");
    broadcastMessage(heartbeat);
}

void ChatServer::broadcastMessage(const QJsonObject &message, QTcpSocket* excludeSocket)
{
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    
    for (const auto &client : clients) {
        if (client.socket != excludeSocket && client.socket->state() == QTcpSocket::ConnectedState) {
            client.socket->write(data);
        }
    }
}

void ChatServer::sendToClient(QTcpSocket* socket, const QJsonObject &message)
{
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        QJsonDocument doc(message);
        QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
        socket->write(data);
    }
}

ClientInfo* ChatServer::findClientBySocket(QTcpSocket* socket)
{
    for (auto &client : clients) {
        if (client.socket == socket) {
            return &client;
        }
    }
    return nullptr;
}

ClientInfo* ChatServer::findClientByNickname(const QString &nickname)
{
    for (auto &client : clients) {
        if (client.nickname == nickname) {
            return &client;
        }
    }
    return nullptr;
}

void ChatServer::removeClient(QTcpSocket* socket)
{
    for (int i = 0; i < clients.size(); ++i) {
        if (clients[i].socket == socket) {
            clients.removeAt(i);
            break;
        }
    }
}

QJsonObject ChatServer::createMessage(const QString &type, const QString &content, const QString &sender)
{
    QJsonObject message;
    message["type"] = type;
    message["content"] = content;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    if (!sender.isEmpty()) {
        message["sender"] = sender;
    }
    return message;
} 