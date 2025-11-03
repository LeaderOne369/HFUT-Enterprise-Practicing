#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    a.setApplicationName("精美浏览器");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("BrowserApp");
    a.setOrganizationDomain("browserapp.com");
    
    MainWindow w;
    w.show();
    
    return a.exec();
}
