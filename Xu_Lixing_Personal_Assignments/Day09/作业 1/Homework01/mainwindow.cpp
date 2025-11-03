#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , itemCounter(0)
{
    ui->setupUi(this);
    
    // 初始化图标路径列表
    iconPaths << ":/icons/folder.png" << ":/icons/file.png" << ":/icons/document.png" 
              << ":/icons/image.png" << ":/icons/music.png" << ":/icons/video.png";
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    applyModernStyle();
    
    // 设置窗口标题和大小
    setWindowTitle("精美树形结构管理器 v1.0");
    resize(1200, 800);
    setMinimumSize(800, 600);
    
    // 状态栏显示欢迎信息
    statusLabel->setText("欢迎使用树形结构管理器！");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧面板 - 操作按钮区域
    QGroupBox *controlGroup = new QGroupBox("操作面板", this);
    controlGroup->setFixedWidth(200);
    
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    
    // 文件操作按钮
    QGroupBox *fileGroup = new QGroupBox("文件操作");
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
    
    openFileBtn = new QPushButton("📁 打开文件");
    saveBtn = new QPushButton("💾 保存数据");
    loadBtn = new QPushButton("📂 加载数据");
    
    fileLayout->addWidget(openFileBtn);
    fileLayout->addWidget(saveBtn);
    fileLayout->addWidget(loadBtn);
    
    // 树操作按钮
    QGroupBox *treeGroup = new QGroupBox("树操作");
    QVBoxLayout *treeLayout = new QVBoxLayout(treeGroup);
    
    addRootBtn = new QPushButton("➕ 添加根节点");
    addChildBtn = new QPushButton("🌿 添加子节点");
    deleteBtn = new QPushButton("🗑️ 删除节点");
    expandAllBtn = new QPushButton("📖 展开全部");
    collapseAllBtn = new QPushButton("📕 折叠全部");
    clearBtn = new QPushButton("🧹 清空树");
    
    treeLayout->addWidget(addRootBtn);
    treeLayout->addWidget(addChildBtn);
    treeLayout->addWidget(deleteBtn);
    treeLayout->addWidget(expandAllBtn);
    treeLayout->addWidget(collapseAllBtn);
    treeLayout->addWidget(clearBtn);
    
    controlLayout->addWidget(fileGroup);
    controlLayout->addWidget(treeGroup);
    controlLayout->addStretch();
    
    // 中间 - 树形结构
    QWidget *treeContainer = new QWidget();
    QVBoxLayout *treeContainerLayout = new QVBoxLayout(treeContainer);
    
    QLabel *treeLabel = new QLabel("🌳 树形结构视图");
    treeLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; margin: 5px;");
    
    setupTreeWidget();
    
    treeContainerLayout->addWidget(treeLabel);
    treeContainerLayout->addWidget(treeWidget);
    
    // 右侧 - 详情显示
    QGroupBox *detailsGroup = new QGroupBox("节点详情");
    detailsGroup->setFixedWidth(300);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);
    
    detailsTextEdit = new QTextEdit();
    detailsTextEdit->setReadOnly(true);
    detailsTextEdit->setText("选择一个树节点查看详细信息...");
    
    detailsLayout->addWidget(detailsTextEdit);
    
    // 组装分割器
    mainSplitter->addWidget(controlGroup);
    mainSplitter->addWidget(treeContainer);
    mainSplitter->addWidget(detailsGroup);
    mainSplitter->setStretchFactor(1, 1);
    
    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(mainSplitter);
    mainLayout->setContentsMargins(10, 10, 10, 10);
}

void MainWindow::setupTreeWidget()
{
    treeWidget = new QTreeWidget();
    treeWidget->setHeaderLabels(QStringList() << "名称" << "类型" << "创建时间");
    treeWidget->setAlternatingRowColors(true);
    treeWidget->setAnimated(true);
    treeWidget->setSortingEnabled(true);
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    treeWidget->setDragDropMode(QAbstractItemView::InternalMove);
    
    // 设置列宽
    treeWidget->setColumnWidth(0, 200);
    treeWidget->setColumnWidth(1, 100);
    treeWidget->setColumnWidth(2, 150);
    
    // 美化树控件
    treeWidget->setStyleSheet(
        "QTreeWidget {"
        "    background-color: #ffffff;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    font-size: 12px;"
        "    selection-background-color: #3498db;"
        "    selection-color: white;"
        "}"
        "QTreeWidget::item {"
        "    height: 30px;"
        "    border-bottom: 1px solid #ecf0f1;"
        "    padding: 2px;"
        "}"
        "QTreeWidget::item:hover {"
        "    background-color: #e8f4fd;"
        "}"
        "QTreeWidget::item:selected {"
        "    background-color: #3498db;"
        "    color: white;"
        "}"
        "QTreeWidget::branch:has-children {"
        "    image: url();"
        "}"
    );
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件");
    fileMenu->addAction("打开文件", this, &MainWindow::openFile, QKeySequence::Open);
    fileMenu->addAction("保存数据", this, &MainWindow::saveTreeData, QKeySequence::Save);
    fileMenu->addAction("加载数据", this, &MainWindow::loadTreeData, QKeySequence("Ctrl+L"));
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &QWidget::close, QKeySequence::Quit);
    
    // 编辑菜单
    QMenu *editMenu = menuBar->addMenu("编辑");
    editMenu->addAction("添加根节点", this, &MainWindow::addRootItem, QKeySequence("Ctrl+R"));
    editMenu->addAction("添加子节点", this, &MainWindow::addChildItem, QKeySequence("Ctrl+N"));
    editMenu->addAction("删除节点", this, &MainWindow::deleteItem, QKeySequence::Delete);
    editMenu->addSeparator();
    editMenu->addAction("重命名", this, &MainWindow::renameItem, QKeySequence("F2"));
    editMenu->addAction("更改图标", this, &MainWindow::changeItemIcon, QKeySequence("Ctrl+I"));
    
    // 视图菜单
    QMenu *viewMenu = menuBar->addMenu("视图");
    viewMenu->addAction("展开全部", this, &MainWindow::expandAll, QKeySequence("Ctrl+E"));
    viewMenu->addAction("折叠全部", this, &MainWindow::collapseAll, QKeySequence("Ctrl+C"));
    viewMenu->addAction("清空树", this, &MainWindow::clearTree, QKeySequence("Ctrl+K"));
    
    // 帮助菜单
    QMenu *helpMenu = menuBar->addMenu("帮助");
    helpMenu->addAction("关于", this, &MainWindow::showAbout);
}

void MainWindow::setupStatusBar()
{
    statusLabel = new QLabel();
    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    progressBar->setMaximumWidth(200);
    
    statusBar()->addWidget(statusLabel, 1);
    statusBar()->addPermanentWidget(progressBar);
}

void MainWindow::setupConnections()
{
    // 按钮连接
    connect(openFileBtn, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveTreeData);
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::loadTreeData);
    connect(addRootBtn, &QPushButton::clicked, this, &MainWindow::addRootItem);
    connect(addChildBtn, &QPushButton::clicked, this, &MainWindow::addChildItem);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteItem);
    connect(expandAllBtn, &QPushButton::clicked, this, &MainWindow::expandAll);
    connect(collapseAllBtn, &QPushButton::clicked, this, &MainWindow::collapseAll);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearTree);
    
    // 树控件连接
    connect(treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onItemDoubleClicked);
    connect(treeWidget, &QTreeWidget::itemChanged, this, &MainWindow::onItemChanged);
    connect(treeWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(treeWidget, &QTreeWidget::itemSelectionChanged, [this]() {
        QTreeWidgetItem *item = treeWidget->currentItem();
        if (item) {
            updateItemDetails(item);
        }
    });
}

void MainWindow::applyModernStyle()
{
    // 设置现代化样式
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f8f9fa;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "    background-color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "    color: #2c3e50;"
        "}"
        "QPushButton {"
        "    background-color: #3498db;"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "    min-height: 30px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
        "QTextEdit {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    background-color: #ffffff;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 11px;"
        "}"
        "QMenuBar {"
        "    background-color: #34495e;"
        "    color: white;"
        "    border: none;"
        "}"
        "QMenuBar::item {"
        "    background-color: transparent;"
        "    padding: 4px 8px;"
        "}"
        "QMenuBar::item:selected {"
        "    background-color: #2c3e50;"
        "}"
        "QMenu {"
        "    background-color: white;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #3498db;"
        "    color: white;"
        "}"
        "QStatusBar {"
        "    background-color: #ecf0f1;"
        "    border-top: 1px solid #bdc3c7;"
        "}"
    );
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "打开文件", QString(), "所有文件 (*.*);;文本文件 (*.txt);;JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        currentFilePath = fileName;
        statusLabel->setText("已打开文件: " + QFileInfo(fileName).fileName());
        
        // 这里可以根据文件类型进行不同的处理
        // 现在简单地添加一个表示文件的根节点
        QTreeWidgetItem *fileItem = new QTreeWidgetItem();
        fileItem->setText(0, QFileInfo(fileName).fileName());
        fileItem->setText(1, "文件");
        fileItem->setText(2, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        fileItem->setIcon(0, getRandomIcon());
        
        treeWidget->addTopLevelItem(fileItem);
        treeWidget->setCurrentItem(fileItem);
    }
}

void MainWindow::saveTreeData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存树数据", "tree_data.json", "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        progressBar->setVisible(true);
        progressBar->setRange(0, 0); // 无限进度条
        
        QJsonArray jsonArray = treeToJson();
        QJsonDocument doc(jsonArray);
        
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
            statusLabel->setText("数据已保存到: " + QFileInfo(fileName).fileName());
            QMessageBox::information(this, "保存成功", "树形数据已成功保存！");
        } else {
            QMessageBox::warning(this, "保存失败", "无法保存文件: " + file.errorString());
        }
        
        progressBar->setVisible(false);
    }
}

void MainWindow::loadTreeData()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "加载树数据", QString(), "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        progressBar->setVisible(true);
        progressBar->setRange(0, 0);
        
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();
            
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                treeWidget->clear();
                loadJsonToTree(doc.array());
                statusLabel->setText("数据已从 " + QFileInfo(fileName).fileName() + " 加载");
                QMessageBox::information(this, "加载成功", "树形数据已成功加载！");
            } else {
                QMessageBox::warning(this, "加载失败", "文件格式不正确！");
            }
        } else {
            QMessageBox::warning(this, "加载失败", "无法读取文件: " + file.errorString());
        }
        
        progressBar->setVisible(false);
    }
}

void MainWindow::addRootItem()
{
    bool ok;
    QString text = QInputDialog::getText(this, "添加根节点",
                                        "节点名称:", QLineEdit::Normal,
                                        "新根节点", &ok);
    if (ok && !text.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, text);
        item->setText(1, "根节点");
        item->setText(2, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        item->setIcon(0, getRandomIcon());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        
        treeWidget->addTopLevelItem(item);
        treeWidget->setCurrentItem(item);
        itemCounter++;
        statusLabel->setText("已添加根节点: " + text);
    }
}

void MainWindow::addChildItem()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个父节点！");
        return;
    }
    
    bool ok;
    QString text = QInputDialog::getText(this, "添加子节点",
                                        "节点名称:", QLineEdit::Normal,
                                        "新子节点", &ok);
    if (ok && !text.isEmpty()) {
        QTreeWidgetItem *childItem = new QTreeWidgetItem();
        childItem->setText(0, text);
        childItem->setText(1, "子节点");
        childItem->setText(2, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        childItem->setIcon(0, getRandomIcon());
        childItem->setFlags(childItem->flags() | Qt::ItemIsEditable);
        
        currentItem->addChild(childItem);
        currentItem->setExpanded(true);
        treeWidget->setCurrentItem(childItem);
        itemCounter++;
        statusLabel->setText("已添加子节点: " + text);
    }
}

void MainWindow::deleteItem()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择要删除的节点！");
        return;
    }
    
    int ret = QMessageBox::question(this, "确认删除",
                                   "确定要删除选中的节点及其所有子节点吗？",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        QTreeWidgetItem *parent = currentItem->parent();
        if (parent) {
            parent->removeChild(currentItem);
        } else {
            int index = treeWidget->indexOfTopLevelItem(currentItem);
            treeWidget->takeTopLevelItem(index);
        }
        delete currentItem;
        statusLabel->setText("节点已删除");
    }
}

void MainWindow::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if (item) {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        treeWidget->editItem(item, 0);
    }
}

void MainWindow::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == 0 && item) {
        statusLabel->setText("节点 '" + item->text(0) + "' 已重命名");
        updateItemDetails(item);
    }
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = treeWidget->itemAt(pos);
    if (!item) return;
    
    QMenu contextMenu(this);
    contextMenu.addAction("🏷️ 重命名", this, &MainWindow::renameItem);
    contextMenu.addAction("🎨 更改图标", this, &MainWindow::changeItemIcon);
    contextMenu.addSeparator();
    contextMenu.addAction("➕ 添加子节点", this, &MainWindow::addChildItem);
    contextMenu.addAction("🗑️ 删除节点", this, &MainWindow::deleteItem);
    
    contextMenu.exec(treeWidget->mapToGlobal(pos));
}

void MainWindow::renameItem()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if (currentItem) {
        treeWidget->editItem(currentItem, 0);
    }
}

void MainWindow::changeItemIcon()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个节点！");
        return;
    }
    
    // 简单的图标选择，这里随机更换
    currentItem->setIcon(0, getRandomIcon());
    statusLabel->setText("已更改节点图标");
}

void MainWindow::expandAll()
{
    treeWidget->expandAll();
    statusLabel->setText("已展开所有节点");
}

void MainWindow::collapseAll()
{
    treeWidget->collapseAll();
    statusLabel->setText("已折叠所有节点");
}

void MainWindow::clearTree()
{
    int ret = QMessageBox::question(this, "确认清空",
                                   "确定要清空整个树吗？此操作无法撤销！",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        treeWidget->clear();
        detailsTextEdit->clear();
        detailsTextEdit->setText("树已清空，选择一个节点查看详细信息...");
        itemCounter = 0;
        statusLabel->setText("树已清空");
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于",
                      "<h3>精美树形结构管理器 v1.0</h3>"
                      "<p>这是一个功能完整的QT树形结构应用程序。</p>"
                      "<p><b>主要功能：</b></p>"
                      "<ul>"
                      "<li>📁 文件读取和树形显示</li>"
                      "<li>🎨 自定义图标支持</li>"
                      "<li>✏️ 快捷重命名操作</li>"
                      "<li>💾 树结构数据保存</li>"
                      "<li>🎯 现代化界面设计</li>"
                      "</ul>"
                      "<p>开发时间：2024年</p>");
}

void MainWindow::loadJsonToTree(const QJsonArray &jsonArray, QTreeWidgetItem *parentItem)
{
    for (const QJsonValue &value : jsonArray) {
        QJsonObject obj = value.toObject();
        
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, obj["name"].toString());
        item->setText(1, obj["type"].toString());
        item->setText(2, obj["time"].toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setIcon(0, getRandomIcon());
        
        if (parentItem) {
            parentItem->addChild(item);
        } else {
            treeWidget->addTopLevelItem(item);
        }
        
        if (obj.contains("children")) {
            loadJsonToTree(obj["children"].toArray(), item);
        }
    }
}

QJsonArray MainWindow::treeToJson(QTreeWidgetItem *parentItem)
{
    QJsonArray jsonArray;
    
    int count = parentItem ? parentItem->childCount() : treeWidget->topLevelItemCount();
    
    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem *item = parentItem ? parentItem->child(i) : treeWidget->topLevelItem(i);
        
        QJsonObject obj;
        obj["name"] = item->text(0);
        obj["type"] = item->text(1);
        obj["time"] = item->text(2);
        
        if (item->childCount() > 0) {
            obj["children"] = treeToJson(item);
        }
        
        jsonArray.append(obj);
    }
    
    return jsonArray;
}

void MainWindow::updateItemDetails(QTreeWidgetItem *item)
{
    if (!item) return;
    
    QString details = QString(
        "<h3>📋 节点详情</h3>"
        "<table style='width: 100%; font-family: Arial;'>"
        "<tr><td><b>名称：</b></td><td>%1</td></tr>"
        "<tr><td><b>类型：</b></td><td>%2</td></tr>"
        "<tr><td><b>创建时间：</b></td><td>%3</td></tr>"
        "<tr><td><b>子节点数：</b></td><td>%4</td></tr>"
        "<tr><td><b>层级：</b></td><td>%5</td></tr>"
        "</table>"
        "<br><h4>🌳 节点路径：</h4>"
        "<p style='background-color: #f0f0f0; padding: 5px; border-radius: 3px;'>"
    ).arg(item->text(0), item->text(1), item->text(2))
     .arg(item->childCount())
     .arg(getItemLevel(item));
    
    // 构建节点路径
    QStringList path;
    QTreeWidgetItem *current = item;
    while (current) {
        path.prepend(current->text(0));
        current = current->parent();
    }
    details += path.join(" → ");
    details += "</p>";
    
    detailsTextEdit->setHtml(details);
}

int MainWindow::getItemLevel(QTreeWidgetItem *item)
{
    int level = 0;
    QTreeWidgetItem *parent = item->parent();
    while (parent) {
        level++;
        parent = parent->parent();
    }
    return level;
}

QIcon MainWindow::getRandomIcon()
{
    // 创建一些简单的彩色图标
    static QStringList iconStyles = {
        "📁", "📄", "🌟", "🎯", "🎨", "🔧", "⚙️", "🌲", "🍃", "💎"
    };
    
    // 这里简化处理，返回空图标，实际应用中可以加载真实的图标文件
    // 或者创建简单的图标
    return QIcon();
}
