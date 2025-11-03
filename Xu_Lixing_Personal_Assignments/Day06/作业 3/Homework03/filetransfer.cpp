#include "filetransfer.h"
#include <QDebug>
#include <QDataStream>

FileTransfer::FileTransfer(QObject *parent)
    : QObject(parent), socket(nullptr), server(nullptr), file(nullptr),
      transferring(false), bytesTransferred(0), totalBytes(0)
{
}

FileTransfer::~FileTransfer()
{
    cancelTransfer();
}

bool FileTransfer::sendFile(const QString &filePath, const QString &host, quint16 port)
{
    if (transferring) {
        emit transferFailed("已有传输正在进行");
        return false;
    }

    file = new QFile(filePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        emit transferFailed(QString("无法打开文件: %1").arg(file->errorString()));
        delete file;
        file = nullptr;
        return false;
    }

    QFileInfo fileInfo(filePath);
    fileName = fileInfo.fileName();
    totalBytes = file->size();
    bytesTransferred = 0;
    mode = Sender;

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &FileTransfer::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &FileTransfer::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &FileTransfer::onErrorOccurred);
    connect(socket, &QTcpSocket::bytesWritten, this, &FileTransfer::onBytesWritten);

    socket->connectToHost(host, port);
    transferring = true;
    return true;
}

bool FileTransfer::receiveFile(const QString &savePath, quint16 port)
{
    if (transferring) {
        emit transferFailed("已有传输正在进行");
        return false;
    }

    this->savePath = savePath;
    mode = Receiver;
    bytesTransferred = 0;

    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &FileTransfer::onNewConnection);

    if (!server->listen(QHostAddress::Any, port)) {
        emit transferFailed(QString("无法监听端口 %1: %2").arg(port).arg(server->errorString()));
        delete server;
        server = nullptr;
        return false;
    }

    transferring = true;
    qDebug() << "等待文件传输连接，端口:" << port;
    return true;
}

void FileTransfer::cancelTransfer()
{
    transferring = false;

    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
        socket = nullptr;
    }

    if (server) {
        server->close();
        server->deleteLater();
        server = nullptr;
    }

    if (file) {
        file->close();
        file->deleteLater();
        file = nullptr;
    }

    resetTransfer();
}

int FileTransfer::getProgress() const
{
    if (totalBytes == 0) return 0;
    return static_cast<int>((bytesTransferred * 100) / totalBytes);
}

void FileTransfer::onConnected()
{
    emit connectionEstablished();
    qDebug() << "连接已建立，开始发送文件头";
    
    if (mode == Sender && !writeFileHeader()) {
        emit transferFailed("发送文件头失败");
        cancelTransfer();
        return;
    }
    
    emit transferStarted();
}

void FileTransfer::onDisconnected()
{
    if (transferring) {
        if (bytesTransferred == totalBytes && totalBytes > 0) {
            emit transferCompleted();
        } else {
            emit transferFailed("连接意外断开");
        }
    }
    resetTransfer();
}

void FileTransfer::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    emit transferFailed(socket->errorString());
    resetTransfer();
}

void FileTransfer::onReadyRead()
{
    if (mode == Receiver) {
        if (totalBytes == 0) {
            // 读取文件头
            if (!readFileHeader()) {
                return;
            }
            emit transferStarted();
            
            // 检查是否还有数据需要处理
            if (socket->bytesAvailable() > 0) {
                // 递归调用处理剩余数据
                onReadyRead();
            }
            return;
        }

        // 读取文件数据
        QByteArray data = socket->readAll();
        if (!data.isEmpty()) {
            if (file && file->write(data) == data.size()) {
                bytesTransferred += data.size();
                emit transferProgress(getProgress(), bytesTransferred, totalBytes);

                if (bytesTransferred >= totalBytes) {
                    qDebug() << "文件接收完成，总接收字节:" << bytesTransferred;
                    emit transferCompleted();
                    cancelTransfer();
                }
            } else {
                emit transferFailed("写入文件失败");
                cancelTransfer();
            }
        }
    }
}

void FileTransfer::onBytesWritten(qint64 bytes)
{
    if (mode == Sender) {
        sendNextChunk();
    }
}

void FileTransfer::onNewConnection()
{
    socket = server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &FileTransfer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &FileTransfer::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &FileTransfer::onErrorOccurred);

    emit connectionEstablished();
    qDebug() << "接收到文件传输连接";
    
    // 关闭服务器，只接受一个连接
    server->close();
}

void FileTransfer::resetTransfer()
{
    transferring = false;
    bytesTransferred = 0;
    totalBytes = 0;
    fileName.clear();
    savePath.clear();
}

void FileTransfer::sendNextChunk()
{
    if (!file || !socket || bytesTransferred >= totalBytes) {
        return;
    }

    QByteArray data = file->read(BUFFER_SIZE);
    if (!data.isEmpty()) {
        qint64 written = socket->write(data);
        if (written == data.size()) {
            bytesTransferred += written;
            emit transferProgress(getProgress(), bytesTransferred, totalBytes);

            if (bytesTransferred >= totalBytes) {
                qDebug() << "文件发送完成";
                socket->flush(); // 确保数据被发送
                socket->disconnectFromHost();
            }
        } else {
            emit transferFailed("发送数据失败");
            cancelTransfer();
        }
    } else if (file->atEnd()) {
        // 文件读取完成
        qDebug() << "文件读取完成，总发送字节:" << bytesTransferred;
        socket->flush();
        socket->disconnectFromHost();
    }
}

bool FileTransfer::writeFileHeader()
{
    if (!socket || !file) return false;

    QByteArray header;
    QDataStream stream(&header, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);

    // 简化文件头格式：文件名长度 + 文件名 + 文件大小
    stream << fileName << totalBytes;

    qDebug() << "准备发送文件头:" << fileName << "大小:" << totalBytes;
    qint64 written = socket->write(header);
    socket->flush();

    if (written == header.size()) {
        qDebug() << "文件头发送成功，头部大小:" << header.size() << "字节";
        sendNextChunk(); // 开始发送文件内容
        return true;
    } else {
        qDebug() << "文件头发送失败: 写入" << written << "字节，期望" << header.size() << "字节";
        return false;
    }
}

bool FileTransfer::readFileHeader()
{
    if (!socket) return false;

    // 检查是否有足够的数据读取基本文件头
    if (socket->bytesAvailable() < 16) { // 至少需要16字节
        qDebug() << "等待更多数据，当前可用:" << socket->bytesAvailable() << "字节";
        return false; // 等待更多数据
    }

    // 尝试读取文件头
    QByteArray headerData = socket->peek(qMin(socket->bytesAvailable(), (qint64)512)); // 预览最多512字节
    QDataStream stream(&headerData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_15);

    QString tempFileName;
    qint64 tempTotalBytes;
    stream >> tempFileName >> tempTotalBytes;

    if (stream.status() != QDataStream::Ok || tempFileName.isEmpty() || tempTotalBytes <= 0) {
        qDebug() << "文件头解析失败: status=" << stream.status() 
                << "fileName=" << tempFileName << "size=" << tempTotalBytes;
        if (socket->bytesAvailable() < 64) { // 如果数据不够，继续等待
            return false;
        }
        emit transferFailed("文件头格式错误");
        return false;
    }

    // 计算实际需要读取的字节数
    qint64 headerSize = stream.device()->pos();
    if (socket->bytesAvailable() < headerSize) {
        qDebug() << "文件头不完整，需要" << headerSize << "字节，当前有" << socket->bytesAvailable() << "字节";
        return false;
    }

    // 正式读取文件头
    socket->read(headerSize);
    fileName = tempFileName;
    totalBytes = tempTotalBytes;

    qDebug() << "文件头读取成功:" << fileName << "大小:" << totalBytes << "头部字节:" << headerSize;

    // 创建文件
    QString fullPath = savePath;
    if (!fullPath.endsWith('/')) {
        fullPath += '/';
    }
    fullPath += fileName;

    file = new QFile(fullPath, this);
    if (!file->open(QIODevice::WriteOnly)) {
        emit transferFailed(QString("无法创建文件: %1").arg(file->errorString()));
        delete file;
        file = nullptr;
        return false;
    }

    qDebug() << "接收文件头成功:" << fileName << "大小:" << totalBytes;
    return true;
}