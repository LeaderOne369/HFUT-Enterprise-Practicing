#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set application properties
    a.setApplicationName("绘图工具");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("Drawing Tools");
    
    // Load translation
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "zh_CN";
        if (translator.load(":/translations/" + baseName + ".qm")) {
            a.installTranslator(&translator);
            break;
        }
    }
    
    MainWindow w;
    w.show();
    
    return a.exec();
}
