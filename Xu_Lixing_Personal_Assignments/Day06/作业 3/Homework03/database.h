#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDateTime>
#include <QDebug>

struct ChatMessage {
    QString sender;
    QString content;
    QDateTime timestamp;
    bool isFile;
    QString filePath;
    qint64 fileSize;
};

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();
    
    bool initialize();
    bool addMessage(const QString &sender, const QString &content, bool isFile = false, 
                   const QString &filePath = "", qint64 fileSize = 0);
    QList<ChatMessage> getMessages(int limit = 100);
    QList<ChatMessage> getMessagesByDate(const QDate &date);
    QList<ChatMessage> getMessagesBySender(const QString &sender);
    bool clearMessages();

private:
    QSqlDatabase db;
    bool createTables();
};

#endif // DATABASE_H 