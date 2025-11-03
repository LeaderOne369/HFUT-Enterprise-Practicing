#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>

class BookmarkManager : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarkManager(QWidget *parent = nullptr);
    void addBookmark(const QString &title, const QUrl &url);
    void loadBookmarks();
    void saveBookmarks();

signals:
    void bookmarkClicked(const QUrl &url);

private slots:
    void onAddBookmark();
    void onDeleteBookmark();
    void onBookmarkDoubleClicked();

private:
    QListWidget *m_bookmarkList;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;
    QSettings *m_settings;
    
    void setupUI();
};

#endif // BOOKMARKMANAGER_H 