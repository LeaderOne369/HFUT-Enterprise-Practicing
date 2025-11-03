#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QFileDialog>
#include <QProgressBar>
#include <QMessageBox>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QHttpMultiPart>
#include <QMimeDatabase>
#include <QUrl>
#include <QDir>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文件浏览槽函数
    void browseDownloadPath();
    void browseUploadFile();
    
    // HTTP操作槽函数
    void startDownload();
    void startUpload();
    void sendPostRequest();
    
    // 网络响应处理槽函数
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void downloadFinished();
    void uploadFinished();
    void postRequestFinished();
    void networkError(QNetworkReply::NetworkError error);
    
    // UI槽函数
    void formatResponse();
    void clearData();
    void showAbout();
    void exitApplication();

private:
    Ui::MainWindow *ui;
    
    // 网络管理器
    QNetworkAccessManager *networkManager;
    
    // 当前活动的网络回复对象
    QNetworkReply *currentDownloadReply;
    QNetworkReply *currentUploadReply;
    QNetworkReply *currentPostReply;
    
    // 文件相关
    QFile *downloadFile;
    QString uploadFilePath;
    
    // 辅助方法
    void setupConnections();
    void updateStatus(const QString &message);
    QString formatJsonString(const QString &jsonString);
    QString formatXmlString(const QString &xmlString);
    void showNetworkError(const QString &operation, QNetworkReply::NetworkError error);
    void resetProgressBars();
    bool validateUrl(const QString &url);
    void enableDownloadControls(bool enabled);
    void enableUploadControls(bool enabled);
    void enablePostControls(bool enabled);
};

#endif // MAINWINDOW_H
