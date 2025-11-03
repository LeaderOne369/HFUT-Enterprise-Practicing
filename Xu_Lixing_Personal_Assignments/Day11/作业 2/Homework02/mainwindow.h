#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QUrl>
#include <QWebEngineHistory>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QSplitter>
#include <QTimer>
#include "bookmarkmanager.h"

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
    void navigateToUrl();
    void goBack();
    void goForward();
    void reload();
    void goHome();
    void updateUrlBar(const QUrl &url);
    void updateLoadProgress(int progress);
    void finishLoading(bool success);
    void showBookmarks();
    void addCurrentBookmark();
    void printPage();
    void printPreview();
    void openBookmarkUrl(const QUrl &url);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void updateNavigationButtons();
    
    Ui::MainWindow *ui;
    QWebEngineView *m_webView;
    QLineEdit *m_urlEdit;
    QPushButton *m_backButton;
    QPushButton *m_forwardButton;
    QPushButton *m_reloadButton;
    QPushButton *m_homeButton;
    QPushButton *m_goButton;
    QPushButton *m_bookmarkButton;
    QProgressBar *m_progressBar;
    QSplitter *m_splitter;
    
    BookmarkManager *m_bookmarkManager;
    
    // 菜单和动作
    QAction *m_printAction;
    QAction *m_printPreviewAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_bookmarkAction;
    QAction *m_addBookmarkAction;
};

#endif // MAINWINDOW_H
