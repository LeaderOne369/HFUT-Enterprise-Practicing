#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QSplitter>
#include <QProgressBar>
#include <QTimer>
#include <QComboBox>
#include <QSpinBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QTime>
#include <QPixmap>
#include <QBuffer>
#include <QImageWriter>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void openFolder();
    void play();
    void pause();
    void stop();
    void previous();
    void next();
    void seek(int position);
    void volumeChanged(int volume);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onItemDoubleClicked(QListWidgetItem *item);
    void savePlaylist();
    void loadPlaylist();
    void clearPlaylist();
    void removeFromPlaylist();
    void takeScreenshot();
    void speedChanged(double speed);
    void updatePosition();
    
    // 日志系统槽函数
    void onClearLog();
    void onSaveLog();
    void onSwitchToLog();

private:
    void setupUI();
    void setupMenuBar();
    void setupConnections();
    void addToPlaylist(const QString &fileName);
    void playCurrentItem();
    void setCurrentItem(int index);
    QString formatTime(qint64 timeInMs);
    bool isSupportedFormat(const QString &fileName);
    
    // 日志系统函数
    void setupLogView();
    void logMessage(const QString &level, const QString &message);
    void logInfo(const QString &message);
    void logWarning(const QString &message);
    void logError(const QString &message);
    void logDebug(const QString &message);
    
    Ui::MainWindow *ui;
    
    // 媒体播放器组件
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    QVideoWidget *videoWidget;
    
    // 播放列表
    QListWidget *playlistWidget;
    QStringList playlist;
    int currentIndex;
    
    // 控制界面
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *openFileButton;
    QPushButton *openFolderButton;
    QPushButton *screenshotButton;
    
    QSlider *positionSlider;
    QSlider *volumeSlider;
    QLabel *currentTimeLabel;
    QLabel *totalTimeLabel;
    QLabel *volumeLabel;
    QLabel *statusLabel;
    
    QComboBox *speedComboBox;
    
    // 布局
    QWidget *centralWidget;
    QSplitter *mainSplitter;
    QWidget *controlWidget;
    QWidget *playlistWidget_container;
    
    QTimer *positionTimer;
    
    // 菜单栏
    QMenu *fileMenu;
    QMenu *playbackMenu;
    QMenu *playlistMenu;
    QMenu *toolsMenu;
    
    QAction *openFileAction;
    QAction *openFolderAction;
    QAction *exitAction;
    QAction *playAction;
    QAction *pauseAction;
    QAction *stopAction;
    QAction *previousAction;
    QAction *nextAction;
    QAction *savePlaylistAction;
    QAction *loadPlaylistAction;
    QAction *clearPlaylistAction;
    QAction *screenshotAction;
    
    // 日志系统
    QTextEdit *logTextEdit;
    QPushButton *clearLogButton;
    QPushButton *saveLogButton;
    QWidget *logWidget;
    QAction *switchToLogAction;
};

#endif // MAINWINDOW_H
