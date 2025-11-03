#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDir>
#include <QStandardPaths>
#include <QKeySequence>
#include <QListWidgetItem>
#include <QTextStream>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QScreen>
#include <QDateTime>
#include <QFileInfo>
#include <QDebug>
#include <QStandardPaths>
#include <QScrollBar>
#include <QDialog>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mediaPlayer(nullptr)
    , audioOutput(nullptr)
    , videoWidget(nullptr)
    , currentIndex(-1)
    , logTextEdit(nullptr)
    , clearLogButton(nullptr)
    , saveLogButton(nullptr)
    , logWidget(nullptr)
{
    ui->setupUi(this);
    setupLogView();
    setupUI();
    setupMenuBar();
    setupConnections();
    
    // 启动日志
    logInfo("=== 精美音视频播放器启动 ===");
    logInfo("版本: 1.0.0");
    logInfo("Qt版本: " + QString(QT_VERSION_STR));
    logInfo("操作系统: macOS");
    
    // 设置窗口属性
    setWindowTitle("精美音视频播放器");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    
    // 启用拖拽
    setAcceptDrops(true);
    
    // 初始化位置定时器
    positionTimer = new QTimer(this);
    connect(positionTimer, &QTimer::timeout, this, &MainWindow::updatePosition);
    positionTimer->start(100); // 每100ms更新一次
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建媒体播放器 - 仿照experiment02的成功实现
    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(0.5f); // 设置初始音量
    mediaPlayer->setAudioOutput(audioOutput);
    
    // 创建视频显示窗口 - 采用experiment02的设置方法
    videoWidget = new QVideoWidget(this);
    videoWidget->setMinimumSize(640, 480);
    videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // macOS 特殊设置 - 来自experiment02
    videoWidget->setAttribute(Qt::WA_OpaquePaintEvent);
    videoWidget->setAttribute(Qt::WA_NoSystemBackground);
    videoWidget->setStyleSheet("background-color: black;");
    videoWidget->show(); // 确保小部件可见
    
    // 设置视频输出
    mediaPlayer->setVideoOutput(videoWidget);
    
    qDebug() << "Media player backend:" << mediaPlayer->metaObject()->className();
    qDebug() << "Video widget backend:" << videoWidget->metaObject()->className();
    
    // 创建播放列表
    playlistWidget = new QListWidget(this);
    playlistWidget->setMaximumWidth(250);
    playlistWidget->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #cccccc;"
        "   border-radius: 5px;"
        "   background-color: #f9f9f9;"
        "}"
        "QListWidget::item {"
        "   padding: 5px;"
        "   border-bottom: 1px solid #eeeeee;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "}"
    );
    
    // 创建控制按钮
    playButton = new QPushButton("▶️", this);
    pauseButton = new QPushButton("⏸️", this);
    stopButton = new QPushButton("⏹️", this);
    previousButton = new QPushButton("⏮️", this);
    nextButton = new QPushButton("⏭️", this);
    openFileButton = new QPushButton("打开文件", this);
    openFolderButton = new QPushButton("打开文件夹", this);
    screenshotButton = new QPushButton("📸截图", this);
    
    // 设置按钮样式
    QString buttonStyle = 
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   border: none;"
        "   color: white;"
        "   padding: 8px 16px;"
        "   text-align: center;"
        "   font-size: 14px;"
        "   border-radius: 5px;"
        "   margin: 2px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d8b40;"
        "}";
    
    playButton->setStyleSheet(buttonStyle);
    pauseButton->setStyleSheet(buttonStyle);
    stopButton->setStyleSheet(buttonStyle);
    previousButton->setStyleSheet(buttonStyle);
    nextButton->setStyleSheet(buttonStyle);
    openFileButton->setStyleSheet(buttonStyle);
    openFolderButton->setStyleSheet(buttonStyle);
    screenshotButton->setStyleSheet(buttonStyle);
    
    // 创建滑块和标签
    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setRange(0, 0);
    positionSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   border: 1px solid #999999;"
        "   height: 8px;"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
        "   margin: 2px 0;"
        "   border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
        "   border: 1px solid #5c5c5c;"
        "   width: 18px;"
        "   margin: -2px 0;"
        "   border-radius: 9px;"
        "}"
    );
    
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setMaximumWidth(100);
    volumeSlider->setStyleSheet(positionSlider->styleSheet());
    
    currentTimeLabel = new QLabel("00:00", this);
    totalTimeLabel = new QLabel("00:00", this);
    volumeLabel = new QLabel("🔊", this);
    statusLabel = new QLabel("就绪", this);
    
    // 创建速度选择器
    speedComboBox = new QComboBox(this);
    speedComboBox->addItems({"0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "2.0x"});
    speedComboBox->setCurrentText("1.0x");
    speedComboBox->setStyleSheet(
        "QComboBox {"
        "   border: 1px solid #cccccc;"
        "   border-radius: 3px;"
        "   padding: 1px 18px 1px 3px;"
        "   min-width: 6em;"
        "}"
    );
    
    // 布局设置
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：视频播放区域
    QWidget *videoContainer = new QWidget(this);
    QVBoxLayout *videoLayout = new QVBoxLayout(videoContainer);
    videoLayout->addWidget(videoWidget);
    
    // 控制面板布局
    controlWidget = new QWidget(this);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
    
    // 位置控制
    QHBoxLayout *positionLayout = new QHBoxLayout();
    positionLayout->addWidget(currentTimeLabel);
    positionLayout->addWidget(positionSlider);
    positionLayout->addWidget(totalTimeLabel);
    
    // 播放控制
    QHBoxLayout *playbackLayout = new QHBoxLayout();
    playbackLayout->addWidget(previousButton);
    playbackLayout->addWidget(playButton);
    playbackLayout->addWidget(pauseButton);
    playbackLayout->addWidget(stopButton);
    playbackLayout->addWidget(nextButton);
    playbackLayout->addStretch();
    playbackLayout->addWidget(new QLabel("速度:"));
    playbackLayout->addWidget(speedComboBox);
    playbackLayout->addWidget(volumeLabel);
    playbackLayout->addWidget(volumeSlider);
    
    // 文件操作
    QHBoxLayout *fileLayout = new QHBoxLayout();
    fileLayout->addWidget(openFileButton);
    fileLayout->addWidget(openFolderButton);
    fileLayout->addWidget(screenshotButton);
    fileLayout->addStretch();
    fileLayout->addWidget(statusLabel);
    
    controlLayout->addLayout(positionLayout);
    controlLayout->addLayout(playbackLayout);
    controlLayout->addLayout(fileLayout);
    
    videoLayout->addWidget(controlWidget);
    
    // 右侧：播放列表
    playlistWidget_container = new QWidget(this);
    QVBoxLayout *playlistLayout = new QVBoxLayout(playlistWidget_container);
    QLabel *playlistLabel = new QLabel("播放列表", this);
    playlistLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    playlistLayout->addWidget(playlistLabel);
    playlistLayout->addWidget(playlistWidget);
    
    // 播放列表按钮
    QHBoxLayout *playlistButtonLayout = new QHBoxLayout();
    QPushButton *savePlaylistBtn = new QPushButton("保存列表", this);
    QPushButton *loadPlaylistBtn = new QPushButton("加载列表", this);
    QPushButton *clearPlaylistBtn = new QPushButton("清空列表", this);
    
    savePlaylistBtn->setStyleSheet(buttonStyle);
    loadPlaylistBtn->setStyleSheet(buttonStyle);
    clearPlaylistBtn->setStyleSheet(buttonStyle);
    
    playlistButtonLayout->addWidget(savePlaylistBtn);
    playlistButtonLayout->addWidget(loadPlaylistBtn);
    playlistButtonLayout->addWidget(clearPlaylistBtn);
    playlistLayout->addLayout(playlistButtonLayout);
    
    connect(savePlaylistBtn, &QPushButton::clicked, this, &MainWindow::savePlaylist);
    connect(loadPlaylistBtn, &QPushButton::clicked, this, &MainWindow::loadPlaylist);
    connect(clearPlaylistBtn, &QPushButton::clicked, this, &MainWindow::clearPlaylist);
    
    mainSplitter->addWidget(videoContainer);
    mainSplitter->addWidget(playlistWidget_container);
    mainSplitter->setSizes({800, 200});
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(mainSplitter);
    
    // 设置初始状态
    pauseButton->setEnabled(false);
    stopButton->setEnabled(false);
    screenshotButton->setEnabled(false);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    fileMenu = menuBar()->addMenu("文件(&F)");
    
    openFileAction = new QAction("打开文件(&O)", this);
    openFileAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openFileAction);
    
    openFolderAction = new QAction("打开文件夹(&D)", this);
    openFolderAction->setShortcut(QKeySequence("Ctrl+D"));
    fileMenu->addAction(openFolderAction);
    
    fileMenu->addSeparator();
    
    exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(exitAction);
    
    // 播放菜单
    playbackMenu = menuBar()->addMenu("播放(&P)");
    
    playAction = new QAction("播放(&P)", this);
    playAction->setShortcut(QKeySequence(Qt::Key_Space));
    playbackMenu->addAction(playAction);
    
    pauseAction = new QAction("暂停(&A)", this);
    // 不设置快捷键，通过playAction来切换
    playbackMenu->addAction(pauseAction);
    
    stopAction = new QAction("停止(&S)", this);
    stopAction->setShortcut(QKeySequence("Ctrl+S"));
    playbackMenu->addAction(stopAction);
    
    playbackMenu->addSeparator();
    
    previousAction = new QAction("上一首(&V)", this);
    previousAction->setShortcut(QKeySequence::MoveToPreviousChar);
    playbackMenu->addAction(previousAction);
    
    nextAction = new QAction("下一首(&N)", this);
    nextAction->setShortcut(QKeySequence::MoveToNextChar);
    playbackMenu->addAction(nextAction);
    
    // 播放列表菜单
    playlistMenu = menuBar()->addMenu("播放列表(&L)");
    
    savePlaylistAction = new QAction("保存播放列表(&S)", this);
    savePlaylistAction->setShortcut(QKeySequence::Save);
    playlistMenu->addAction(savePlaylistAction);
    
    loadPlaylistAction = new QAction("加载播放列表(&L)", this);
    loadPlaylistAction->setShortcut(QKeySequence("Ctrl+L"));
    playlistMenu->addAction(loadPlaylistAction);
    
    clearPlaylistAction = new QAction("清空播放列表(&C)", this);
    clearPlaylistAction->setShortcut(QKeySequence("Ctrl+E"));
    playlistMenu->addAction(clearPlaylistAction);
    
    // 工具菜单
    toolsMenu = menuBar()->addMenu("工具(&T)");
    
    screenshotAction = new QAction("截图(&S)", this);
    screenshotAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    toolsMenu->addAction(screenshotAction);
    
    toolsMenu->addSeparator();
    
    switchToLogAction = new QAction("查看日志(&L)", this);
    switchToLogAction->setShortcut(QKeySequence("Ctrl+L"));
    toolsMenu->addAction(switchToLogAction);
}

void MainWindow::setupConnections()
{
    // 按钮连接
    connect(playButton, &QPushButton::clicked, this, &MainWindow::play);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::pause);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(previousButton, &QPushButton::clicked, this, &MainWindow::previous);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::next);
    connect(openFileButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(openFolderButton, &QPushButton::clicked, this, &MainWindow::openFolder);
    connect(screenshotButton, &QPushButton::clicked, this, &MainWindow::takeScreenshot);
    
    // 滑块连接
    connect(positionSlider, &QSlider::sliderMoved, this, &MainWindow::seek);
    connect(volumeSlider, &QSlider::valueChanged, this, &MainWindow::volumeChanged);
    
    // 播放器连接
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::positionChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::durationChanged);
    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::mediaStatusChanged);
    connect(mediaPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        QString msg = QString("播放器错误 [%1]: %2").arg(error).arg(errorString);
        statusLabel->setText(msg);
        QMessageBox::critical(this, "播放器错误", msg);
    });
    
    // 播放列表连接
    connect(playlistWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onItemDoubleClicked);
    
    // 速度控制连接
    connect(speedComboBox, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            [this](const QString &text) {
                QString speedText = text;
                speedText.remove('x');
                double speed = speedText.toDouble();
                speedChanged(speed);
            });
    
    // 菜单动作连接
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::openFolder);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(playAction, &QAction::triggered, this, &MainWindow::play);
    connect(pauseAction, &QAction::triggered, this, &MainWindow::pause);
    connect(stopAction, &QAction::triggered, this, &MainWindow::stop);
    connect(previousAction, &QAction::triggered, this, &MainWindow::previous);
    connect(nextAction, &QAction::triggered, this, &MainWindow::next);
    connect(savePlaylistAction, &QAction::triggered, this, &MainWindow::savePlaylist);
    connect(loadPlaylistAction, &QAction::triggered, this, &MainWindow::loadPlaylist);
    connect(clearPlaylistAction, &QAction::triggered, this, &MainWindow::clearPlaylist);
    connect(screenshotAction, &QAction::triggered, this, &MainWindow::takeScreenshot);
    connect(switchToLogAction, &QAction::triggered, this, &MainWindow::onSwitchToLog);
}

void MainWindow::openFile()
{
    logInfo("用户点击打开文件按钮");
    
    QString fileName = QFileDialog::getOpenFileName(this,
        "打开媒体文件", 
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
        "媒体文件 (*.mp3 *.wav *.mp4 *.avi *.mkv *.mov *.wmv *.flv *.m4a *.aac *.ogg *.webm);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        logInfo(QString("用户选择了媒体文件: %1").arg(fileName));
        
        // 验证文件是否存在和可读
        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists()) {
            logError("选择的文件不存在");
            QMessageBox::warning(this, "文件错误", "选择的文件不存在！");
            return;
        }
        
        if (!fileInfo.isReadable()) {
            logError("选择的文件不可读");
            QMessageBox::warning(this, "文件错误", "无法读取选择的文件！请检查文件权限。");
            return;
        }
        
        // 检查文件大小
        qint64 fileSize = fileInfo.size();
        if (fileSize == 0) {
            logError("选择的文件为空");
            QMessageBox::warning(this, "文件错误", "选择的文件为空！");
            return;
        }
        
        logInfo(QString("文件验证通过 - 大小: %1 MB").arg(fileSize / 1024.0 / 1024.0, 0, 'f', 2));
        
        // 停止当前播放
        if (mediaPlayer->playbackState() != QMediaPlayer::StoppedState) {
            logDebug("停止当前播放");
            mediaPlayer->stop();
        }
        
        addToPlaylist(fileName);
        setCurrentItem(playlist.size() - 1);
        playCurrentItem();
    } else {
        logDebug("用户取消了文件选择");
    }
}

void MainWindow::openFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹");
    if (folderPath.isEmpty()) return;
    
    QDir dir(folderPath);
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.mp4" << "*.avi" << "*.mkv" << "*.mov" 
            << "*.wmv" << "*.flv" << "*.m4a" << "*.aac" << "*.ogg" << "*.webm";
    
    QStringList files = dir.entryList(filters, QDir::Files);
    for (const QString &file : files) {
        addToPlaylist(dir.absoluteFilePath(file));
    }
    
    if (!files.isEmpty()) {
        statusLabel->setText(QString("已添加 %1 个文件").arg(files.size()));
    }
}

void MainWindow::play()
{
    logInfo("用户点击播放按钮");
    
    if (mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
        logDebug("从暂停状态恢复播放");
        mediaPlayer->play();
    } else if (currentIndex >= 0 && currentIndex < playlist.size()) {
        logDebug("开始播放新项目");
        playCurrentItem();
    } else {
        logWarning("没有可播放的项目");
        return;
    }
    
    playButton->setEnabled(false);
    pauseButton->setEnabled(true);
    stopButton->setEnabled(true);
}

void MainWindow::pause()
{
    logInfo("用户点击暂停按钮");
    mediaPlayer->pause();
    playButton->setEnabled(true);
    pauseButton->setEnabled(false);
}

void MainWindow::stop()
{
    logInfo("用户点击停止按钮");
    mediaPlayer->stop();
    playButton->setEnabled(true);
    pauseButton->setEnabled(false);
    stopButton->setEnabled(false);
    positionSlider->setValue(0);
    currentTimeLabel->setText("00:00");
}

void MainWindow::previous()
{
    if (currentIndex > 0) {
        setCurrentItem(currentIndex - 1);
        playCurrentItem();
    }
}

void MainWindow::next()
{
    if (currentIndex < playlist.size() - 1) {
        setCurrentItem(currentIndex + 1);
        playCurrentItem();
    }
}

void MainWindow::seek(int position)
{
    mediaPlayer->setPosition(position);
}

void MainWindow::volumeChanged(int volume)
{
    audioOutput->setVolume(volume / 100.0);
    volumeLabel->setText(volume == 0 ? "🔇" : (volume < 50 ? "🔉" : "🔊"));
}

void MainWindow::positionChanged(qint64 position)
{
    if (!positionSlider->isSliderDown()) {
        positionSlider->setValue(position);
    }
    currentTimeLabel->setText(formatTime(position));
}

void MainWindow::durationChanged(qint64 duration)
{
    positionSlider->setRange(0, duration);
    totalTimeLabel->setText(formatTime(duration));
}

void MainWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    QString statusText;
    switch (status) {
    case QMediaPlayer::NoMedia:
        statusText = "无媒体";
        break;
    case QMediaPlayer::LoadingMedia:
        statusText = "正在加载...";
        statusLabel->setText("正在加载视频文件...");
        break;
    case QMediaPlayer::LoadedMedia:
        {
            statusText = "加载完成";
            statusLabel->setText("视频加载完成");
            screenshotButton->setEnabled(true);
            playButton->setEnabled(true);
            
                    // 播放一帧来显示视频预览
        logDebug("开始播放预览帧");
        mediaPlayer->play();
        QTimer *previewTimer = new QTimer(this);
        previewTimer->setSingleShot(true);
        connect(previewTimer, &QTimer::timeout, this, [this, previewTimer]() {
            if (mediaPlayer && mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
                mediaPlayer->pause();
                logDebug("预览帧显示完成，暂停播放");
            }
            previewTimer->deleteLater();
        });
        previewTimer->start(100);
            break;
        }
    case QMediaPlayer::StalledMedia:
        statusText = "媒体暂停";
        break;
    case QMediaPlayer::BufferingMedia:
        statusText = "正在缓冲...";
        break;
    case QMediaPlayer::BufferedMedia:
        statusText = "缓冲完成";
        break;
    case QMediaPlayer::EndOfMedia:
        statusText = "播放结束";
        next(); // 自动播放下一首
        break;
    case QMediaPlayer::InvalidMedia:
        statusText = "无效媒体";
        statusLabel->setText("无效媒体文件");
        // 输出错误信息
        if (mediaPlayer->error() != QMediaPlayer::NoError) {
            QString errorMsg = QString("媒体错误: %1").arg(mediaPlayer->errorString());
            statusLabel->setText(errorMsg);
            QMessageBox::warning(this, "播放错误", errorMsg);
        }
        break;
    }
    logDebug(QString("媒体状态变化: %1").arg(statusText));
}

void MainWindow::onItemDoubleClicked(QListWidgetItem *item)
{
    int index = playlistWidget->row(item);
    setCurrentItem(index);
    playCurrentItem();
}

void MainWindow::savePlaylist()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存播放列表", "playlist.m3u", "M3U 播放列表 (*.m3u)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "#EXTM3U\n";
        for (const QString &filePath : playlist) {
            stream << filePath << "\n";
        }
        file.close();
        statusLabel->setText("播放列表已保存");
    }
}

void MainWindow::loadPlaylist()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "加载播放列表", "", "M3U 播放列表 (*.m3u);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        clearPlaylist();
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith("#")) {
                if (QFile::exists(line)) {
                    addToPlaylist(line);
                }
            }
        }
        file.close();
        statusLabel->setText(QString("已加载 %1 个文件").arg(playlist.size()));
    }
}

void MainWindow::clearPlaylist()
{
    playlist.clear();
    playlistWidget->clear();
    currentIndex = -1;
    stop();
}

void MainWindow::removeFromPlaylist()
{
    int currentRow = playlistWidget->currentRow();
    if (currentRow >= 0) {
        playlist.removeAt(currentRow);
        delete playlistWidget->takeItem(currentRow);
        
        if (currentIndex == currentRow) {
            currentIndex = -1;
            stop();
        } else if (currentIndex > currentRow) {
            currentIndex--;
        }
    }
}

void MainWindow::takeScreenshot()
{
    if (mediaPlayer->hasVideo()) {
        QPixmap screenshot = videoWidget->grab();
        if (!screenshot.isNull()) {
            QString fileName = QFileDialog::getSaveFileName(this,
                "保存截图", 
                QString("screenshot_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
                "PNG 图片 (*.png);;JPEG 图片 (*.jpg)");
            
            if (!fileName.isEmpty()) {
                if (screenshot.save(fileName)) {
                    statusLabel->setText("截图已保存");
                } else {
                    statusLabel->setText("截图保存失败");
                }
            }
        }
    }
}

void MainWindow::speedChanged(double speed)
{
    mediaPlayer->setPlaybackRate(speed);
    statusLabel->setText(QString("播放速度: %1x").arg(speed));
}

void MainWindow::updatePosition()
{
    // 这个函数可以用于更新界面状态
}

void MainWindow::addToPlaylist(const QString &fileName)
{
    logDebug(QString("尝试添加文件到播放列表: %1").arg(fileName));
    
    if (isSupportedFormat(fileName)) {
        playlist.append(fileName);
        QFileInfo fileInfo(fileName);
        playlistWidget->addItem(fileInfo.baseName());
        logInfo(QString("文件已添加到播放列表: %1").arg(fileInfo.baseName()));
        logDebug(QString("当前播放列表大小: %1").arg(playlist.size()));
    } else {
        logError(QString("不支持的文件格式: %1").arg(fileName));
    }
}

void MainWindow::playCurrentItem()
{
    if (currentIndex >= 0 && currentIndex < playlist.size()) {
        QString filePath = playlist[currentIndex];
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        
        logInfo(QString("尝试播放文件: %1").arg(filePath));
        logDebug(QString("文件URL: %1").arg(fileUrl.toString()));
        logDebug(QString("文件是否存在: %1").arg(QFile::exists(filePath) ? "是" : "否"));
        
        // 安全检查：确保媒体播放器存在
        if (!mediaPlayer) {
            logError("媒体播放器未初始化");
            QMessageBox::critical(this, "内部错误", "媒体播放器未正确初始化！");
            return;
        }
        
        // 停止当前播放
        if (mediaPlayer->playbackState() != QMediaPlayer::StoppedState) {
            logDebug("停止当前播放");
            mediaPlayer->stop();
        }
        
        // 重置UI状态
        playButton->setEnabled(false);
        pauseButton->setEnabled(false);
        positionSlider->setValue(0);
        currentTimeLabel->setText("00:00");
        
        // 设置媒体源
        try {
            mediaPlayer->setSource(fileUrl);
            logDebug(QString("设置媒体源: %1").arg(fileUrl.toString()));
        } catch (...) {
            logError("设置媒体源时发生异常");
            QMessageBox::critical(this, "错误", "媒体文件加载失败！");
            return;
        }
        
        // 检查是否是视频文件
        QFileInfo fileInfo(filePath);
        QString suffix = fileInfo.suffix().toLower();
        bool isVideo = (suffix == "mp4" || suffix == "avi" || suffix == "mkv" || 
                       suffix == "mov" || suffix == "wmv" || suffix == "flv" || suffix == "webm");
        
        if (isVideo) {
            logDebug("检测到视频文件，确保视频组件可见");
            videoWidget->show();
        } else {
            logDebug("检测到音频文件");
        }
        
        // 更新播放列表选中项
        playlistWidget->setCurrentRow(currentIndex);
        
        // 更新文件信息显示
        qint64 fileSize = fileInfo.size();
        QString sizeText = QString::number(fileSize / 1024.0 / 1024.0, 'f', 1) + " MB";
        statusLabel->setText(QString("已选择: %1 (%2)").arg(fileInfo.fileName(), sizeText));
        
        logInfo(QString("媒体文件加载请求已发送: %1").arg(fileInfo.fileName()));
        
        // 注意：播放控制按钮的启用将由mediaStatusChanged信号处理
    }
}

void MainWindow::setCurrentItem(int index)
{
    logDebug(QString("设置当前项目索引: %1, 播放列表大小: %2").arg(index).arg(playlist.size()));
    
    if (index >= 0 && index < playlist.size()) {
        currentIndex = index;
        playlistWidget->setCurrentRow(index);
        logInfo(QString("当前项目已设置为: %1").arg(index));
    } else {
        logError(QString("无效的播放列表索引: %1").arg(index));
    }
}

QString MainWindow::formatTime(qint64 timeInMs)
{
    int seconds = timeInMs / 1000;
    int minutes = seconds / 60;
    seconds %= 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

bool MainWindow::isSupportedFormat(const QString &fileName)
{
    QStringList supportedFormats = {
        "mp3", "wav", "mp4", "avi", "mkv", "mov", 
        "wmv", "flv", "m4a", "aac", "ogg", "webm"
    };
    
    QFileInfo fileInfo(fileName);
    QString fileSuffix = fileInfo.suffix().toLower();
    bool isSupported = supportedFormats.contains(fileSuffix, Qt::CaseInsensitive);
    
    logDebug(QString("检查文件格式: %1, 后缀: %2, 支持的格式: %3, 结果: %4")
             .arg(fileName)
             .arg(fileSuffix)
             .arg(supportedFormats.join(", "))
             .arg(isSupported ? "是" : "否"));
    
    return isSupported;
}

// 日志系统实现
void MainWindow::setupLogView()
{
    logWidget = new QWidget(this);
    QVBoxLayout *logLayout = new QVBoxLayout(logWidget);
    logLayout->setContentsMargins(10, 10, 10, 10);
    logLayout->setSpacing(10);
    
    // 日志标题
    QLabel *logTitle = new QLabel("📋 系统日志", logWidget);
    logTitle->setStyleSheet("font-weight: bold; font-size: 16px; color: #1a73e8; padding: 5px;");
    logLayout->addWidget(logTitle);
    
    // 日志文本框
    logTextEdit = new QTextEdit(logWidget);
    logTextEdit->setReadOnly(true);
    logTextEdit->setStyleSheet(
        "QTextEdit {"
        "   border: 1px solid #cccccc;"
        "   border-radius: 5px;"
        "   background-color: #f8f9fa;"
        "   font-family: 'Courier New', monospace;"
        "   font-size: 12px;"
        "   padding: 8px;"
        "}"
    );
    logLayout->addWidget(logTextEdit);
    
    // 日志操作按钮
    QHBoxLayout *logButtonLayout = new QHBoxLayout();
    
    clearLogButton = new QPushButton("清空日志", logWidget);
    saveLogButton = new QPushButton("保存日志", logWidget);
    
    QString logButtonStyle = 
        "QPushButton {"
        "   background-color: #6c757d;"
        "   border: none;"
        "   color: white;"
        "   padding: 8px 16px;"
        "   text-align: center;"
        "   font-size: 12px;"
        "   border-radius: 4px;"
        "   margin: 2px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5a6268;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #545b62;"
        "}";
    
    clearLogButton->setStyleSheet(logButtonStyle);
    saveLogButton->setStyleSheet(logButtonStyle);
    
    logButtonLayout->addWidget(clearLogButton);
    logButtonLayout->addWidget(saveLogButton);
    logButtonLayout->addStretch();
    
    logLayout->addLayout(logButtonLayout);
    
    // 连接信号
    connect(clearLogButton, &QPushButton::clicked, this, &MainWindow::onClearLog);
    connect(saveLogButton, &QPushButton::clicked, this, &MainWindow::onSaveLog);
}

void MainWindow::logMessage(const QString &level, const QString &message)
{
    if (!logTextEdit) return;
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logEntry = QString("[%1] [%2] %3").arg(timestamp, level, message);
    
    logTextEdit->append(logEntry);
    
    // 自动滚动到底部
    QScrollBar *scrollBar = logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    // 同时输出到调试控制台
    qDebug() << logEntry;
}

void MainWindow::logInfo(const QString &message)
{
    logMessage("INFO", message);
}

void MainWindow::logWarning(const QString &message)
{
    logMessage("WARN", message);
}

void MainWindow::logError(const QString &message)
{
    logMessage("ERROR", message);
}

void MainWindow::logDebug(const QString &message)
{
    logMessage("DEBUG", message);
}

void MainWindow::onClearLog()
{
    if (logTextEdit) {
        logTextEdit->clear();
        logInfo("日志已清空");
    }
}

void MainWindow::onSaveLog()
{
    if (!logTextEdit) return;
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存日志文件", 
        QString("player_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << logTextEdit->toPlainText();
            file.close();
            logInfo(QString("日志已保存到: %1").arg(fileName));
        } else {
            logError("保存日志文件失败");
        }
    }
}

void MainWindow::onSwitchToLog()
{
    // 创建日志窗口
    QDialog *logDialog = new QDialog(this);
    logDialog->setWindowTitle("系统日志");
    logDialog->setMinimumSize(800, 600);
    logDialog->resize(1000, 700);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(logDialog);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    
    // 复制日志组件到对话框
    QWidget *logCopy = new QWidget(logDialog);
    QVBoxLayout *logCopyLayout = new QVBoxLayout(logCopy);
    
    QLabel *title = new QLabel("📋 系统日志", logCopy);
    title->setStyleSheet("font-weight: bold; font-size: 16px; color: #1a73e8; padding: 10px;");
    
    QTextEdit *logCopyEdit = new QTextEdit(logCopy);
    logCopyEdit->setReadOnly(true);
    logCopyEdit->setPlainText(logTextEdit->toPlainText());
    logCopyEdit->setStyleSheet(logTextEdit->styleSheet());
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *refreshBtn = new QPushButton("刷新", logCopy);
    QPushButton *exportBtn = new QPushButton("导出", logCopy);
    QPushButton *closeBtn = new QPushButton("关闭", logCopy);
    
    QString btnStyle = 
        "QPushButton {"
        "   background-color: #007bff;"
        "   border: none;"
        "   color: white;"
        "   padding: 8px 20px;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0056b3;"
        "}";
    
    refreshBtn->setStyleSheet(btnStyle);
    exportBtn->setStyleSheet(btnStyle);
    closeBtn->setStyleSheet(btnStyle);
    
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    
    logCopyLayout->addWidget(title);
    logCopyLayout->addWidget(logCopyEdit);
    logCopyLayout->addLayout(buttonLayout);
    
    dialogLayout->addWidget(logCopy);
    
    // 连接信号
    connect(refreshBtn, &QPushButton::clicked, [this, logCopyEdit]() {
        logCopyEdit->setPlainText(logTextEdit->toPlainText());
        QScrollBar *scrollBar = logCopyEdit->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    });
    
    connect(exportBtn, &QPushButton::clicked, [this, logCopyEdit]() {
        QString fileName = QFileDialog::getSaveFileName(this,
            "导出日志", 
            QString("debug_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
            "文本文件 (*.txt)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                stream << logCopyEdit->toPlainText();
                file.close();
                logInfo(QString("调试日志已导出到: %1").arg(fileName));
            }
        }
    });
    
    connect(closeBtn, &QPushButton::clicked, logDialog, &QDialog::accept);
    
    logInfo("打开日志查看窗口");
    logDialog->exec();
    logDialog->deleteLater();
}
