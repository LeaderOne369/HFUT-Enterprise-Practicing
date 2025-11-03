#include "database.h"
#include <QStandardPaths>
#include <QDir>

Database::Database(QObject *parent)
    : QObject(parent)
{
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::initialize()
{
    // 获取应用数据目录
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dataPath + "/chat.db");
    
    if (!db.open()) {
        qDebug() << "无法打开数据库:" << db.lastError().text();
        return false;
    }
    
    return createTables();
}

bool Database::createTables()
{
    QSqlQuery query(db);
    
    // 创建聊天消息表
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS chat_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            content TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            is_file BOOLEAN DEFAULT FALSE,
            file_path TEXT,
            file_size INTEGER
        )
    )";
    
    if (!query.exec(createTableQuery)) {
        qDebug() << "创建表失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::addMessage(const QString &sender, const QString &content, bool isFile, 
                         const QString &filePath, qint64 fileSize)
{
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO chat_messages (sender, content, timestamp, is_file, file_path, file_size)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(sender);
    query.addBindValue(content);
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(isFile);
    query.addBindValue(filePath);
    query.addBindValue(fileSize);
    
    if (!query.exec()) {
        qDebug() << "插入消息失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<ChatMessage> Database::getMessages(int limit)
{
    QList<ChatMessage> messages;
    QSqlQuery query(db);
    
    query.prepare("SELECT sender, content, timestamp, is_file, file_path, file_size "
                  "FROM chat_messages ORDER BY timestamp DESC LIMIT ?");
    query.addBindValue(limit);
    
    if (!query.exec()) {
        qDebug() << "查询消息失败:" << query.lastError().text();
        return messages;
    }
    
    while (query.next()) {
        ChatMessage msg;
        msg.sender = query.value(0).toString();
        msg.content = query.value(1).toString();
        msg.timestamp = query.value(2).toDateTime();
        msg.isFile = query.value(3).toBool();
        msg.filePath = query.value(4).toString();
        msg.fileSize = query.value(5).toLongLong();
        messages.prepend(msg); // 时间正序
    }
    
    return messages;
}

QList<ChatMessage> Database::getMessagesByDate(const QDate &date)
{
    QList<ChatMessage> messages;
    QSqlQuery query(db);
    
    query.prepare("SELECT sender, content, timestamp, is_file, file_path, file_size "
                  "FROM chat_messages WHERE DATE(timestamp) = ? ORDER BY timestamp");
    query.addBindValue(date.toString("yyyy-MM-dd"));
    
    if (!query.exec()) {
        qDebug() << "按日期查询消息失败:" << query.lastError().text();
        return messages;
    }
    
    while (query.next()) {
        ChatMessage msg;
        msg.sender = query.value(0).toString();
        msg.content = query.value(1).toString();
        msg.timestamp = query.value(2).toDateTime();
        msg.isFile = query.value(3).toBool();
        msg.filePath = query.value(4).toString();
        msg.fileSize = query.value(5).toLongLong();
        messages.append(msg);
    }
    
    return messages;
}

QList<ChatMessage> Database::getMessagesBySender(const QString &sender)
{
    QList<ChatMessage> messages;
    QSqlQuery query(db);
    
    query.prepare("SELECT sender, content, timestamp, is_file, file_path, file_size "
                  "FROM chat_messages WHERE sender = ? ORDER BY timestamp");
    query.addBindValue(sender);
    
    if (!query.exec()) {
        qDebug() << "按发送者查询消息失败:" << query.lastError().text();
        return messages;
    }
    
    while (query.next()) {
        ChatMessage msg;
        msg.sender = query.value(0).toString();
        msg.content = query.value(1).toString();
        msg.timestamp = query.value(2).toDateTime();
        msg.isFile = query.value(3).toBool();
        msg.filePath = query.value(4).toString();
        msg.fileSize = query.value(5).toLongLong();
        messages.append(msg);
    }
    
    return messages;
}

bool Database::clearMessages()
{
    QSqlQuery query(db);
    if (!query.exec("DELETE FROM chat_messages")) {
        qDebug() << "清空消息失败:" << query.lastError().text();
        return false;
    }
    return true;
} 