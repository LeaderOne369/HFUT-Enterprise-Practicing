#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QTextStream>
#include <QStandardPaths>
#include <QAbstractItemView>
#include <QDateTime>
#include <QFile>
#include <QIODevice>

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
    // XML 功能槽函数
    void loadXmlFile();
    void saveXmlFile();
    void createSampleXml();
    void onXmlTreeItemChanged(QTreeWidgetItem *item, int column);
    void onXmlTextChanged();
    
    // JSON 功能槽函数
    void loadJsonFile();
    void saveJsonFile();
    void formatJson();
    void validateJson();
    void onJsonTextChanged();
    
    // MySQL 功能槽函数
    void connectToDatabase();
    void disconnectFromDatabase();
    void executeQuery();
    void clearQuery();

private:
    Ui::MainWindow *ui;
    
    // XML 相关
    QDomDocument xmlDocument;
    QString currentXmlFilePath;
    void populateXmlTree(const QDomElement &element, QTreeWidgetItem *parentItem = nullptr);
    void updateXmlFromTree();
    void updateXmlText();
    
    // JSON 相关
    QJsonDocument jsonDocument;
    QString currentJsonFilePath;
    void populateJsonTree(const QJsonValue &value, QTreeWidgetItem *parentItem = nullptr, const QString &key = "");
    void updateJsonText();
    
    // MySQL 相关
    QSqlDatabase database;
    bool isConnected;
    void setupDatabase();
    void displayQueryResults(const QSqlQuery &query);
    void logMessage(const QString &message);
};

#endif // MAINWINDOW_H
