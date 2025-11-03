#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QApplication>
#include <QScreen>

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

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onMinimizeClicked();
    void onMaximizeClicked();
    void onCloseClicked();
    void onThemeChanged();
    void onBackgroundColorChanged();
    void onBackgroundImageChanged();
    void toggleFullScreen();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void showFromTray();
    void exitApplication();

private:
    void setupUI();
    void setupTrayIcon();
    void setupCustomTitleBar();
    void setupThemeSelector();
    void applyTheme(const QString &themeName);
    void setBackgroundColor(const QColor &color);
    void setBackgroundImage(const QString &imagePath);

    Ui::MainWindow *ui;
    
    // 自定义标题栏
    QWidget *titleBar;
    QLabel *titleLabel;
    QPushButton *minimizeButton;
    QPushButton *maximizeButton;
    QPushButton *closeButton;
    
    // 托盘图标
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *showAction;
    QAction *quitAction;
    
    // 主界面控件
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QGroupBox *themeGroup;
    QComboBox *themeComboBox;
    QPushButton *backgroundColorButton;
    QPushButton *backgroundImageButton;
    QPushButton *fullScreenButton;
    QCheckBox *enableTrayCheckBox;
    
    // 窗口拖拽
    QPoint dragPosition;
    bool isDragging;
    
    // 样式和背景
    QString currentTheme;
    QColor backgroundColor;
    QString backgroundImagePath;
    bool isMaximized;
};
#endif // MAINWINDOW_H
