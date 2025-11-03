#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QHostAddress>

class FileTransfer : public QObject
{
    Q_OBJECT

public:
    enum TransferMode {
        Sender,
        Receiver
    };

    explicit FileTransfer(QObject *parent = nullptr);
    ~FileTransfer();

    // 发送文件
    bool sendFile(const QString &filePath, const QString &host, quint16 port);
    
    // 接收文件
    bool receiveFile(const QString &savePath, quint16 port);
    
    // 取消传输
    void cancelTransfer();
    
    // 获取传输状态
    bool isTransferring() const { return transferring; }
    qint64 getBytesTransferred() const { return bytesTransferred; }
    qint64 getTotalBytes() const { return totalBytes; }
    int getProgress() const;

signals:
    void transferStarted();
    void transferProgress(int percentage, qint64 bytesTransferred, qint64 totalBytes);
    void transferCompleted();
    void transferFailed(const QString &error);
    void connectionEstablished();

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onNewConnection();

private:
    QTcpSocket* socket;
    QTcpServer* server;
    QFile* file;
    TransferMode mode;
    
    bool transferring;
    qint64 bytesTransferred;
    qint64 totalBytes;
    QString fileName;
    QString savePath;
    
    static const int BUFFER_SIZE = 64 * 1024; // 64KB buffer
    
    void resetTransfer();
    void sendNextChunk();
    bool writeFileHeader();
    bool readFileHeader();
};

#endif // FILETRANSFER_H 