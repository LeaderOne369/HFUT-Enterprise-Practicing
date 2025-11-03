#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMessageBox>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_drawingWidget(nullptr)
    , m_translator(new QTranslator(this))
    , m_currentLanguage("zh_CN")
{
    ui->setupUi(this);
    setupUI();
    createActions();
    setupMenus();
    setupToolBar();
    setupStatusBar();
    retranslateUI();
    
    // Set window properties
    setWindowTitle(tr("绘图工具"));
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Apply modern styling
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QToolBar {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
            spacing: 3px;
            padding: 5px;
        }
        QToolBar::separator {
            background-color: #dcdcdc;
            width: 1px;
            margin: 5px;
        }
        QPushButton {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #e6f3ff;
            border-color: #4da6ff;
        }
        QPushButton:pressed {
            background-color: #cce6ff;
        }
        QPushButton:checked {
            background-color: #4da6ff;
            color: white;
            border-color: #4da6ff;
        }
        QSpinBox {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
            border-radius: 4px;
            padding: 5px;
            font-size: 14px;
            min-width: 60px;
        }
        QMenuBar {
            background-color: #ffffff;
            border-bottom: 1px solid #dcdcdc;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
        }
        QMenuBar::item:selected {
            background-color: #e6f3ff;
        }
        QMenu {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
        }
        QMenu::item {
            padding: 8px 32px;
        }
        QMenu::item:selected {
            background-color: #e6f3ff;
        }
        QStatusBar {
            background-color: #ffffff;
            border-top: 1px solid #dcdcdc;
        }
    )");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI() {
    // Create drawing widget
    m_drawingWidget = new DrawingWidget(this);
    setCentralWidget(m_drawingWidget);
    
    // Connect signals
    connect(m_drawingWidget, &DrawingWidget::selectionChanged,
            this, &MainWindow::onSelectionChanged);
}

void MainWindow::createActions() {
    // Drawing mode actions
    m_selectAction = new QAction(tr("选择"), this);
    m_selectAction->setIcon(QIcon(":/icons/select.png"));
    m_selectAction->setCheckable(true);
    m_selectAction->setChecked(true);
    m_selectAction->setShortcut(QKeySequence("S"));
    
    m_rectangleAction = new QAction(tr("矩形"), this);
    m_rectangleAction->setIcon(QIcon(":/icons/rectangle.png"));
    m_rectangleAction->setCheckable(true);
    m_rectangleAction->setShortcut(QKeySequence("R"));
    
    m_circleAction = new QAction(tr("圆形"), this);
    m_circleAction->setIcon(QIcon(":/icons/circle.png"));
    m_circleAction->setCheckable(true);
    m_circleAction->setShortcut(QKeySequence("C"));
    
    m_triangleAction = new QAction(tr("三角形"), this);
    m_triangleAction->setIcon(QIcon(":/icons/triangle.png"));
    m_triangleAction->setCheckable(true);
    m_triangleAction->setShortcut(QKeySequence("T"));
    
    m_polygonAction = new QAction(tr("多边形"), this);
    m_polygonAction->setIcon(QIcon(":/icons/polygon.png"));
    m_polygonAction->setCheckable(true);
    m_polygonAction->setShortcut(QKeySequence("P"));
    
    // Group drawing mode actions
    m_drawModeGroup = new QButtonGroup(this);
    m_drawModeGroup->addButton(new QToolButton(), 0); // Select
    m_drawModeGroup->addButton(new QToolButton(), 1); // Rectangle
    m_drawModeGroup->addButton(new QToolButton(), 2); // Circle
    m_drawModeGroup->addButton(new QToolButton(), 3); // Triangle
    m_drawModeGroup->addButton(new QToolButton(), 4); // Polygon
    m_drawModeGroup->setExclusive(true);
    
    connect(m_selectAction, &QAction::triggered, this, &MainWindow::onDrawModeChanged);
    connect(m_rectangleAction, &QAction::triggered, this, &MainWindow::onDrawModeChanged);
    connect(m_circleAction, &QAction::triggered, this, &MainWindow::onDrawModeChanged);
    connect(m_triangleAction, &QAction::triggered, this, &MainWindow::onDrawModeChanged);
    connect(m_polygonAction, &QAction::triggered, this, &MainWindow::onDrawModeChanged);
    
    // Edit actions
    m_clearAction = new QAction(tr("清空画布"), this);
    m_clearAction->setIcon(QIcon(":/icons/clear.png"));
    m_clearAction->setShortcut(QKeySequence::New);
    connect(m_clearAction, &QAction::triggered, this, &MainWindow::onClearAll);
    
    m_deleteAction = new QAction(tr("删除选中"), this);
    m_deleteAction->setIcon(QIcon(":/icons/delete.png"));
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setEnabled(false);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    
    // File actions
    m_exitAction = new QAction(tr("退出"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Language actions
    m_chineseAction = new QAction(tr("简体中文"), this);
    m_chineseAction->setCheckable(true);
    m_chineseAction->setChecked(true);
    connect(m_chineseAction, &QAction::triggered, this, &MainWindow::onLanguageChanged);
    
    m_englishAction = new QAction(tr("English"), this);
    m_englishAction->setCheckable(true);
    connect(m_englishAction, &QAction::triggered, this, &MainWindow::onLanguageChanged);
    
    QActionGroup *languageGroup = new QActionGroup(this);
    languageGroup->addAction(m_chineseAction);
    languageGroup->addAction(m_englishAction);
}

void MainWindow::setupMenus() {
    // File menu
    m_fileMenu = menuBar()->addMenu(tr("文件"));
    m_fileMenu->addAction(m_clearAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);
    
    // Edit menu
    m_editMenu = menuBar()->addMenu(tr("编辑"));
    m_editMenu->addAction(m_deleteAction);
    
    // Tool menu
    m_toolMenu = menuBar()->addMenu(tr("工具"));
    m_toolMenu->addAction(m_selectAction);
    m_toolMenu->addSeparator();
    m_toolMenu->addAction(m_rectangleAction);
    m_toolMenu->addAction(m_circleAction);
    m_toolMenu->addAction(m_triangleAction);
    m_toolMenu->addAction(m_polygonAction);
    
    // Language menu
    m_languageMenu = menuBar()->addMenu(tr("语言"));
    m_languageMenu->addAction(m_chineseAction);
    m_languageMenu->addAction(m_englishAction);
    
    // Help menu
    m_helpMenu = menuBar()->addMenu(tr("帮助"));
    QAction *aboutAction = m_helpMenu->addAction(tr("关于"));
    connect(aboutAction, &QAction::triggered, [this]() {
        bool isEnglish = (m_currentLanguage == "en_US");
        QString title = isEnglish ? "About" : "关于";
        QString message = isEnglish ? 
            "Drawing Tool v1.0\n\nSupports drawing rectangles, circles, triangles and polygons\nSupports shape editing and multi-language switching" :
            "绘图工具 v1.0\n\n支持绘制矩形、圆形、三角形和多边形\n支持图形编辑和多语言切换";
        QMessageBox::about(this, title, message);
    });
}

void MainWindow::setupToolBar() {
    // Drawing toolbar
    m_drawToolBar = addToolBar(tr("绘图工具"));
    m_drawToolBar->setMovable(false);
    
    // Add drawing mode buttons
    QToolButton *selectBtn = new QToolButton();
    selectBtn->setDefaultAction(m_selectAction);
    selectBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_drawToolBar->addWidget(selectBtn);
    
    m_drawToolBar->addSeparator();
    
    QToolButton *rectBtn = new QToolButton();
    rectBtn->setDefaultAction(m_rectangleAction);
    rectBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_drawToolBar->addWidget(rectBtn);
    
    QToolButton *circleBtn = new QToolButton();
    circleBtn->setDefaultAction(m_circleAction);
    circleBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_drawToolBar->addWidget(circleBtn);
    
    QToolButton *triangleBtn = new QToolButton();
    triangleBtn->setDefaultAction(m_triangleAction);
    triangleBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_drawToolBar->addWidget(triangleBtn);
    
    QToolButton *polygonBtn = new QToolButton();
    polygonBtn->setDefaultAction(m_polygonAction);
    polygonBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_drawToolBar->addWidget(polygonBtn);
    
    // Edit toolbar
    m_editToolBar = addToolBar(tr("编辑工具"));
    m_editToolBar->setMovable(false);
    
    // Color button
    m_colorButton = new QPushButton();
    m_colorButton->setFixedSize(40, 30);
    m_colorButton->setStyleSheet("background-color: black; border: 1px solid gray;");
    m_colorButton->setToolTip(tr("选择颜色"));
    connect(m_colorButton, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);
    
    m_editToolBar->addWidget(new QLabel(tr("颜色: ")));
    m_editToolBar->addWidget(m_colorButton);
    m_editToolBar->addSeparator();
    
    // Line width
    m_lineWidthLabel = new QLabel(tr("线宽: "));
    m_editToolBar->addWidget(m_lineWidthLabel);
    
    m_lineWidthSpinBox = new QSpinBox();
    m_lineWidthSpinBox->setRange(1, 20);
    m_lineWidthSpinBox->setValue(2);
    connect(m_lineWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onLineWidthChanged);
    m_editToolBar->addWidget(m_lineWidthSpinBox);
    
    m_editToolBar->addSeparator();
    m_editToolBar->addAction(m_clearAction);
    m_editToolBar->addAction(m_deleteAction);
}

void MainWindow::setupStatusBar() {
    m_statusLabel = new QLabel("就绪");
    statusBar()->addWidget(m_statusLabel);
    
    // Add language indicator
    QLabel *langLabel = new QLabel();
    langLabel->setText("语言: 简体中文");
    statusBar()->addPermanentWidget(langLabel);
}

void MainWindow::onDrawModeChanged() {
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;
    
    bool isEnglish = (m_currentLanguage == "en_US");
    
    if (action == m_selectAction) {
        m_drawingWidget->setDrawMode(DrawMode::Select);
        m_statusLabel->setText(isEnglish ? "Select Mode" : "选择模式");
    } else if (action == m_rectangleAction) {
        m_drawingWidget->setDrawMode(DrawMode::Rectangle);
        m_statusLabel->setText(isEnglish ? "Draw Rectangle" : "绘制矩形");
    } else if (action == m_circleAction) {
        m_drawingWidget->setDrawMode(DrawMode::Circle);
        m_statusLabel->setText(isEnglish ? "Draw Circle" : "绘制圆形");
    } else if (action == m_triangleAction) {
        m_drawingWidget->setDrawMode(DrawMode::Triangle);
        m_statusLabel->setText(isEnglish ? "Draw Triangle - Click three points" : "绘制三角形 - 点击三个点");
    } else if (action == m_polygonAction) {
        m_drawingWidget->setDrawMode(DrawMode::Polygon);
        m_statusLabel->setText(isEnglish ? "Draw Polygon - Double click to finish" : "绘制多边形 - 双击完成");
    }
}

void MainWindow::onColorButtonClicked() {
    QColor currentColor = m_drawingWidget->hasSelection() ? 
        m_drawingWidget->getSelectedColor() : Qt::black;
    
    bool isEnglish = (m_currentLanguage == "en_US");
    QString title = isEnglish ? "Select Color" : "选择颜色";
    
    QColor color = QColorDialog::getColor(currentColor, this, title);
    if (color.isValid()) {
        m_drawingWidget->setCurrentColor(color);
        m_colorButton->setStyleSheet(
            QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    }
}

void MainWindow::onLineWidthChanged(int width) {
    m_drawingWidget->setLineWidth(width);
}

void MainWindow::onClearAll() {
    bool isEnglish = (m_currentLanguage == "en_US");
    QString title = isEnglish ? "Confirm" : "确认";
    QString message = isEnglish ? "Are you sure you want to clear the canvas? All shapes will be deleted." 
                                : "确定要清空画布吗？所有图形将被删除。";
    
    int ret = QMessageBox::question(this, title, message, QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_drawingWidget->clearAll();
        m_statusLabel->setText(isEnglish ? "Canvas cleared" : "画布已清空");
    }
}

void MainWindow::onDeleteSelected() {
    m_drawingWidget->deleteSelected();
    bool isEnglish = (m_currentLanguage == "en_US");
    m_statusLabel->setText(isEnglish ? "Selected shape deleted" : "选中图形已删除");
}

void MainWindow::onSelectionChanged(bool hasSelection) {
    m_deleteAction->setEnabled(hasSelection);
    
    bool isEnglish = (m_currentLanguage == "en_US");
    
    if (hasSelection) {
        // Update color button and line width to match selected shape
        QColor selectedColor = m_drawingWidget->getSelectedColor();
        m_colorButton->setStyleSheet(
            QString("background-color: %1; border: 1px solid gray;").arg(selectedColor.name()));
        
        int selectedWidth = m_drawingWidget->getSelectedLineWidth();
        m_lineWidthSpinBox->setValue(selectedWidth);
        
        m_statusLabel->setText(isEnglish ? "Shape selected" : "图形已选中");
    } else {
        m_statusLabel->setText(isEnglish ? "Ready" : "就绪");
    }
}

void MainWindow::onLanguageChanged() {
    QAction *action = qobject_cast<QAction*>(sender());
    if (action == m_chineseAction) {
        switchLanguage("zh_CN");
    } else if (action == m_englishAction) {
        switchLanguage("en_US");
    }
}

void MainWindow::switchLanguage(const QString& language) {
    if (m_currentLanguage == language) return;
    
    m_currentLanguage = language;
    
    // For now, we'll just update the UI text directly
    // In a full implementation, you would load proper translation files
    retranslateUI();
}

void MainWindow::retranslateUI() {
    bool isEnglish = (m_currentLanguage == "en_US");
    
    // Update window title
    setWindowTitle(isEnglish ? "Drawing Tool" : "绘图工具");
    
    // Update actions
    m_selectAction->setText(isEnglish ? "Select" : "选择");
    m_rectangleAction->setText(isEnglish ? "Rectangle" : "矩形");
    m_circleAction->setText(isEnglish ? "Circle" : "圆形");
    m_triangleAction->setText(isEnglish ? "Triangle" : "三角形");
    m_polygonAction->setText(isEnglish ? "Polygon" : "多边形");
    m_clearAction->setText(isEnglish ? "Clear Canvas" : "清空画布");
    m_deleteAction->setText(isEnglish ? "Delete Selected" : "删除选中");
    m_exitAction->setText(isEnglish ? "Exit" : "退出");
    m_chineseAction->setText("简体中文");
    m_englishAction->setText("English");
    
    // Update menus
    m_fileMenu->setTitle(isEnglish ? "File" : "文件");
    m_editMenu->setTitle(isEnglish ? "Edit" : "编辑");
    m_toolMenu->setTitle(isEnglish ? "Tools" : "工具");
    m_languageMenu->setTitle(isEnglish ? "Language" : "语言");
    m_helpMenu->setTitle(isEnglish ? "Help" : "帮助");
    
    // Update toolbars
    m_drawToolBar->setWindowTitle(isEnglish ? "Drawing Tools" : "绘图工具");
    m_editToolBar->setWindowTitle(isEnglish ? "Edit Tools" : "编辑工具");
    
    // Update other UI elements
    if (m_lineWidthLabel) {
        m_lineWidthLabel->setText(isEnglish ? "Line Width: " : "线宽: ");
    }
    
    // Update tooltips
    m_colorButton->setToolTip(isEnglish ? "Select Color" : "选择颜色");
    
    // Update status
    QString currentStatus = m_statusLabel->text();
    if (currentStatus == "就绪" || currentStatus == "Ready") {
        m_statusLabel->setText(isEnglish ? "Ready" : "就绪");
    } else if (currentStatus == "选择模式" || currentStatus == "Select Mode") {
        m_statusLabel->setText(isEnglish ? "Select Mode" : "选择模式");
    }
    
    // Update all other toolbar labels
    QList<QLabel*> toolbarLabels = m_editToolBar->findChildren<QLabel*>();
    for (QLabel* label : toolbarLabels) {
        if (label->text().contains("颜色") || label->text().contains("Color")) {
            label->setText(isEnglish ? "Color: " : "颜色: ");
        }
    }
}
