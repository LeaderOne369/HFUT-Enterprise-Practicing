#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QHeaderView>
#include <QSplitter>
#include <QTextEdit>
#include <QGroupBox>
#include <QGridLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>

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
    void openFile();
    void saveTreeData();
    void loadTreeData();
    void addRootItem();
    void addChildItem();
    void deleteItem();
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onItemChanged(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint &pos);
    void renameItem();
    void changeItemIcon();
    void expandAll();
    void collapseAll();
    void clearTree();
    void showAbout();

private:
    Ui::MainWindow *ui;
    QTreeWidget *treeWidget;
    QTextEdit *detailsTextEdit;
    QPushButton *openFileBtn;
    QPushButton *saveBtn;
    QPushButton *loadBtn;
    QPushButton *addRootBtn;
    QPushButton *addChildBtn;
    QPushButton *deleteBtn;
    QPushButton *expandAllBtn;
    QPushButton *collapseAllBtn;
    QPushButton *clearBtn;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    void setupTreeWidget();
    void applyModernStyle();
    
    void loadJsonToTree(const QJsonArray &jsonArray, QTreeWidgetItem *parentItem = nullptr);
    QJsonArray treeToJson(QTreeWidgetItem *parentItem = nullptr);
    void updateItemDetails(QTreeWidgetItem *item);
    QIcon getRandomIcon();
    int getItemLevel(QTreeWidgetItem *item);
    
    QString currentFilePath;
    QStringList iconPaths;
    int itemCounter;
};

#endif // MAINWINDOW_H
