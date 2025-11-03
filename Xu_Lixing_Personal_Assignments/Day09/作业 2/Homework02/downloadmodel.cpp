#include "downloadmodel.h"
#include <QColor>
#include <QRandomGenerator>

DownloadModel::DownloadModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DownloadModel::updateProgress);
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_downloads.size();
}

int DownloadModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_downloads.size())
        return QVariant();

    const DownloadItem &item = m_downloads.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case FileNameColumn:
            return item.fileName;
        case FileSizeColumn:
            return formatSize(item.fileSize);
        case ProgressColumn:
            return QString("%1%").arg(item.progress);
        case SpeedColumn:
            return item.status == Downloading ? formatSpeed(item.speed) : QString("-");
        case RemainingTimeColumn:
            return item.status == Downloading && item.remainingTime > 0 ? 
                   formatTime(item.remainingTime) : QString("-");
        case StatusColumn:
            return statusToString(item.status);
        }
    } else if (role == Qt::UserRole) {
        // 用于进度条委托
        if (index.column() == ProgressColumn) {
            return item.progress;
        }
    } else if (role == Qt::BackgroundRole) {
        switch (item.status) {
        case Completed:
            return QColor(144, 238, 144); // Light green
        case Error:
            return QColor(255, 182, 193); // Light pink
        case Downloading:
            return QColor(173, 216, 230); // Light blue
        default:
            return QVariant();
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == ProgressColumn || index.column() == SpeedColumn || 
            index.column() == RemainingTimeColumn) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QVariant DownloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case FileNameColumn:
            return QString("文件名");
        case FileSizeColumn:
            return QString("文件大小");
        case ProgressColumn:
            return QString("进度");
        case SpeedColumn:
            return QString("下载速度");
        case RemainingTimeColumn:
            return QString("剩余时间");
        case StatusColumn:
            return QString("状态");
        }
    }
    return QVariant();
}

bool DownloadModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        DownloadItem &item = m_downloads[index.row()];
        
        switch (index.column()) {
        case FileNameColumn:
            item.fileName = value.toString();
            break;
        case FileSizeColumn:
            item.fileSize = value.toLongLong();
            break;
        default:
            return false;
        }
        
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags DownloadModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // 只有文件名和文件大小列可以编辑
    if (index.column() == FileNameColumn || index.column() == FileSizeColumn) {
        flags |= Qt::ItemIsEditable;
    }
    
    return flags;
}

void DownloadModel::addDownload(const QString &fileName, qint64 fileSize)
{
    beginInsertRows(QModelIndex(), m_downloads.size(), m_downloads.size());
    m_downloads.append(DownloadItem(fileName, fileSize));
    endInsertRows();
}

void DownloadModel::updateDownload(int row, qint64 downloadedSize, double speed)
{
    if (row < 0 || row >= m_downloads.size())
        return;

    DownloadItem &item = m_downloads[row];
    item.downloadedSize = downloadedSize;
    item.speed = speed;
    
    if (item.fileSize > 0) {
        item.progress = (downloadedSize * 100) / item.fileSize;
        
        if (speed > 0) {
            qint64 remainingBytes = item.fileSize - downloadedSize;
            item.remainingTime = remainingBytes / speed;
        }
        
        if (item.progress >= 100) {
            item.status = Completed;
            item.progress = 100;
            item.speed = 0;
            item.remainingTime = 0;
        } else if (item.status == Waiting) {
            item.status = Downloading;
        }
    }

    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, ColumnCount - 1);
    emit dataChanged(topLeft, bottomRight);
}

void DownloadModel::removeDownload(int row)
{
    if (row < 0 || row >= m_downloads.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_downloads.removeAt(row);
    endRemoveRows();
}

void DownloadModel::clearAll()
{
    beginResetModel();
    m_downloads.clear();
    endResetModel();
}

void DownloadModel::startSimulation()
{
    m_timer->start(1000); // 每秒更新一次
}

void DownloadModel::stopSimulation()
{
    m_timer->stop();
}

void DownloadModel::updateProgress()
{
    for (int i = 0; i < m_downloads.size(); ++i) {
        DownloadItem &item = m_downloads[i];
        
        if (item.status == Downloading) {
            // 模拟下载进度
            double speedVariation = 0.5 + (QRandomGenerator::global()->generateDouble() * 1.5); // 0.5-2.0
            double baseSpeed = 1024 * 1024 * speedVariation; // 0.5-2 MB/s
            
            qint64 increment = baseSpeed; // 每秒增加的字节数
            qint64 newDownloadedSize = qMin(item.downloadedSize + increment, item.fileSize);
            
            updateDownload(i, newDownloadedSize, baseSpeed);
        }
    }
}

QString DownloadModel::formatSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes >= GB)
        return QString("%1 GB").arg(static_cast<double>(bytes) / GB, 0, 'f', 2);
    else if (bytes >= MB)
        return QString("%1 MB").arg(static_cast<double>(bytes) / MB, 0, 'f', 2);
    else if (bytes >= KB)
        return QString("%1 KB").arg(static_cast<double>(bytes) / KB, 0, 'f', 2);
    else
        return QString("%1 B").arg(bytes);
}

QString DownloadModel::formatSpeed(double bytesPerSecond) const
{
    return formatSize(static_cast<qint64>(bytesPerSecond)) + "/s";
}

QString DownloadModel::formatTime(int seconds) const
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    if (hours > 0)
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
    else
        return QString("%1:%2").arg(minutes).arg(secs, 2, 10, QChar('0'));
}

QString DownloadModel::statusToString(DownloadStatus status) const
{
    switch (status) {
    case Waiting:
        return QString("等待中");
    case Downloading:
        return QString("下载中");
    case Paused:
        return QString("已暂停");
    case Completed:
        return QString("已完成");
    case Error:
        return QString("错误");
    default:
        return QString("未知");
    }
} 