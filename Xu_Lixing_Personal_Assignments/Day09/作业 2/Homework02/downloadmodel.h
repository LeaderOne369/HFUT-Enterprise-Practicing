#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractTableModel>
#include <QTimer>
#include <QDateTime>

enum DownloadStatus {
    Waiting,
    Downloading,
    Paused,
    Completed,
    Error
};

struct DownloadItem {
    QString fileName;
    qint64 fileSize;
    qint64 downloadedSize;
    int progress;
    double speed;
    int remainingTime;
    DownloadStatus status;
    QDateTime startTime;
    
    DownloadItem(const QString &name = "", qint64 size = 0)
        : fileName(name), fileSize(size), downloadedSize(0), 
          progress(0), speed(0.0), remainingTime(0), 
          status(Waiting), startTime(QDateTime::currentDateTime()) {}
};

class DownloadModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        FileNameColumn = 0,
        FileSizeColumn,
        ProgressColumn,
        SpeedColumn,
        RemainingTimeColumn,
        StatusColumn,
        ColumnCount
    };

    explicit DownloadModel(QObject *parent = nullptr);
    
    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Custom methods
    void addDownload(const QString &fileName, qint64 fileSize);
    void updateDownload(int row, qint64 downloadedSize, double speed);
    void removeDownload(int row);
    void clearAll();
    void startSimulation();
    void stopSimulation();

private slots:
    void updateProgress();

private:
    QList<DownloadItem> m_downloads;
    QTimer *m_timer;
    QString formatSize(qint64 bytes) const;
    QString formatSpeed(double bytesPerSecond) const;
    QString formatTime(int seconds) const;
    QString statusToString(DownloadStatus status) const;
};

#endif // DOWNLOADMODEL_H 