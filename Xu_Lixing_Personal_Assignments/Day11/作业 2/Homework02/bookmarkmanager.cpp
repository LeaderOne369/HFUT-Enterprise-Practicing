#include "bookmarkmanager.h"

BookmarkManager::BookmarkManager(QWidget *parent)
    : QWidget(parent)
    , m_settings(new QSettings("BrowserApp", "Bookmarks", this))
{
    setupUI();
    loadBookmarks();
}

void BookmarkManager::setupUI()
{
    setWindowTitle("书签管理");
    setFixedSize(400, 300);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 书签列表
    m_bookmarkList = new QListWidget;
    m_bookmarkList->setAlternatingRowColors(true);
    mainLayout->addWidget(m_bookmarkList);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_addButton = new QPushButton("添加书签");
    m_deleteButton = new QPushButton("删除书签");
    
    m_addButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; padding: 8px; border-radius: 4px; }");
    m_deleteButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; padding: 8px; border-radius: 4px; }");
    
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(m_addButton, &QPushButton::clicked, this, &BookmarkManager::onAddBookmark);
    connect(m_deleteButton, &QPushButton::clicked, this, &BookmarkManager::onDeleteBookmark);
    connect(m_bookmarkList, &QListWidget::itemDoubleClicked, this, &BookmarkManager::onBookmarkDoubleClicked);
}

void BookmarkManager::addBookmark(const QString &title, const QUrl &url)
{
    QString bookmarkText = QString("%1 - %2").arg(title, url.toString());
    
    // 检查是否已存在
    for (int i = 0; i < m_bookmarkList->count(); ++i) {
        if (m_bookmarkList->item(i)->data(Qt::UserRole).toUrl() == url) {
            QMessageBox::information(this, "提示", "该网页已在书签中！");
            return;
        }
    }
    
    QListWidgetItem *item = new QListWidgetItem(bookmarkText);
    item->setData(Qt::UserRole, url);
    item->setToolTip(url.toString());
    m_bookmarkList->addItem(item);
    
    saveBookmarks();
}

void BookmarkManager::loadBookmarks()
{
    int size = m_settings->beginReadArray("bookmarks");
    for (int i = 0; i < size; ++i) {
        m_settings->setArrayIndex(i);
        QString title = m_settings->value("title").toString();
        QUrl url = m_settings->value("url").toUrl();
        
        QString bookmarkText = QString("%1 - %2").arg(title, url.toString());
        QListWidgetItem *item = new QListWidgetItem(bookmarkText);
        item->setData(Qt::UserRole, url);
        item->setToolTip(url.toString());
        m_bookmarkList->addItem(item);
    }
    m_settings->endArray();
}

void BookmarkManager::saveBookmarks()
{
    m_settings->beginWriteArray("bookmarks");
    for (int i = 0; i < m_bookmarkList->count(); ++i) {
        m_settings->setArrayIndex(i);
        QListWidgetItem *item = m_bookmarkList->item(i);
        QString text = item->text();
        QUrl url = item->data(Qt::UserRole).toUrl();
        
        QString title = text.split(" - ").first();
        m_settings->setValue("title", title);
        m_settings->setValue("url", url);
    }
    m_settings->endArray();
}

void BookmarkManager::onAddBookmark()
{
    bool ok;
    QString title = QInputDialog::getText(this, "添加书签", "书签名称:", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;
    
    QString urlString = QInputDialog::getText(this, "添加书签", "网址:", QLineEdit::Normal, "https://", &ok);
    if (!ok || urlString.isEmpty()) return;
    
    QUrl url(urlString);
    if (!url.isValid()) {
        QMessageBox::warning(this, "错误", "无效的网址！");
        return;
    }
    
    addBookmark(title, url);
}

void BookmarkManager::onDeleteBookmark()
{
    int currentRow = m_bookmarkList->currentRow();
    if (currentRow >= 0) {
        delete m_bookmarkList->takeItem(currentRow);
        saveBookmarks();
    }
}

void BookmarkManager::onBookmarkDoubleClicked()
{
    QListWidgetItem *item = m_bookmarkList->currentItem();
    if (item) {
        QUrl url = item->data(Qt::UserRole).toUrl();
        emit bookmarkClicked(url);
        hide();
    }
} 