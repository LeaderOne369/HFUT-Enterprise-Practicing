#include <QtWidgets/QApplication>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include <QStringList>

bool isSupportedFormat(const QString &fileName)
{
    QStringList supportedFormats = {
        ".mp3", ".wav", ".mp4", ".avi", ".mkv", ".mov", 
        ".wmv", ".flv", ".m4a", ".aac", ".ogg", ".webm"
    };
    
    QFileInfo fileInfo(fileName);
    QString fileSuffix = fileInfo.suffix().toLower();
    bool isSupported = supportedFormats.contains("." + fileSuffix, Qt::CaseInsensitive);
    
    qDebug() << QString("检查文件格式: %1, 后缀: .%2, 支持: %3")
                .arg(fileName)
                .arg(fileSuffix)
                .arg(isSupported ? "是" : "否");
    
    return isSupported;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 测试文件路径
    QString testFile = "/Users/leaderone/柴静回应《看见》被禁：真实自有万钧之力.mp4";
    
    qDebug() << "=== 开始调试测试 ===";
    qDebug() << "测试文件:" << testFile;
    
    QFileInfo fileInfo(testFile);
    qDebug() << "文件存在:" << fileInfo.exists();
    qDebug() << "文件可读:" << fileInfo.isReadable();
    qDebug() << "文件大小:" << fileInfo.size() << "字节";
    qDebug() << "文件后缀:" << fileInfo.suffix();
    qDebug() << "文件基名:" << fileInfo.baseName();
    
    bool supported = isSupportedFormat(testFile);
    qDebug() << "文件格式支持:" << supported;
    
    qDebug() << "=== 测试完成 ===";
    
    return 0;
} 