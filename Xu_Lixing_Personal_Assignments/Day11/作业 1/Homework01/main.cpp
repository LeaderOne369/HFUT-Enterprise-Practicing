#include "mainwindow.h"
#include <QApplication>
#include <QMediaPlayer>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序属性
    a.setApplicationName("精美音视频播放器");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("MediaPlayer");
    
    // 调试Qt多媒体信息
    qDebug() << "Qt版本:" << QT_VERSION_STR;
    qDebug() << "应用程序目录:" << QCoreApplication::applicationDirPath();
    
    // 检查多媒体后端
    QMediaPlayer *testPlayer = new QMediaPlayer();
    qDebug() << "媒体播放器已创建";
    
    // 设置库路径
    QStringList libraryPaths = QCoreApplication::libraryPaths();
    qDebug() << "Qt插件路径:" << libraryPaths;
    
    // 检查插件目录
    QString pluginPath = QCoreApplication::applicationDirPath() + "/../PlugIns";
    if (QDir(pluginPath).exists()) {
        qDebug() << "插件目录存在:" << pluginPath;
        QStringList plugins = QDir(pluginPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        qDebug() << "可用插件:" << plugins;
    }
    
    delete testPlayer;
    
    MainWindow w;
    w.show();
    return a.exec();
}
