#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>
#include "downloadmodel.h"
#include "progressbardelegate.h"

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
    void addDownload();
    void removeDownload();
    void clearAllDownloads();
    void startDownloads();
    void stopDownloads();

private:
    void setupUI();
    void setupConnections();

    Ui::MainWindow *ui;
    QTableView *m_tableView;
    DownloadModel *m_downloadModel;
    ProgressBarDelegate *m_progressDelegate;
    
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QPushButton *m_clearButton;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
};

#endif // MAINWINDOW_H
