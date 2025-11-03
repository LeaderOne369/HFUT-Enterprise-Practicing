#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QActionGroup>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QTranslator>
#include <QButtonGroup>
#include "drawingwidget.h"

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
    void onDrawModeChanged();
    void onColorButtonClicked();
    void onLineWidthChanged(int width);
    void onClearAll();
    void onDeleteSelected();
    void onSelectionChanged(bool hasSelection);
    void onLanguageChanged();

private:
    void setupUI();
    void setupMenus();
    void setupToolBar();
    void setupStatusBar();
    void createActions();
    void retranslateUI();
    void switchLanguage(const QString& language);
    
    Ui::MainWindow *ui;
    DrawingWidget *m_drawingWidget;
    
    // Toolbar and menus
    QToolBar *m_drawToolBar;
    QToolBar *m_editToolBar;
    QMenuBar *m_menuBar;
    QStatusBar *m_statusBar;
    
    // Actions
    QAction *m_selectAction;
    QAction *m_rectangleAction;
    QAction *m_circleAction;
    QAction *m_triangleAction;
    QAction *m_polygonAction;
    QAction *m_clearAction;
    QAction *m_deleteAction;
    QAction *m_exitAction;
    
    // Language actions
    QAction *m_chineseAction;
    QAction *m_englishAction;
    
    // UI elements
    QButtonGroup *m_drawModeGroup;
    QPushButton *m_colorButton;
    QSpinBox *m_lineWidthSpinBox;
    QLabel *m_statusLabel;
    QLabel *m_lineWidthLabel;
    
    // Translation
    QTranslator *m_translator;
    QString m_currentLanguage;
    
    // Menus
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_toolMenu;
    QMenu *m_languageMenu;
    QMenu *m_helpMenu;
};

#endif // MAINWINDOW_H
