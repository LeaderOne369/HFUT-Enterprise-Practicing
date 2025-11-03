#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
    , currentDownloadReply(nullptr)
    , currentUploadReply(nullptr)
    , currentPostReply(nullptr)
    , downloadFile(nullptr)
{
    ui->setupUi(this);
    setupConnections();
    resetProgressBars();
    updateStatus("就绪");
    
    // 设置默认示例数据
    ui->downloadUrlEdit->setText("https://httpbin.org/json");
    ui->uploadUrlEdit->setText("https://httpbin.org/post");
    ui->postUrlEdit->setText("https://httpbin.org/post");
    ui->postDataEdit->setPlainText("{\n    \"name\": \"测试用户\",\n    \"age\": 25,\n    \"city\": \"北京\"\n}");
}

MainWindow::~MainWindow()
{
    if (downloadFile) {
        downloadFile->close();
        delete downloadFile;
    }
    delete ui;
}

void MainWindow::setupConnections()
{
    // 文件浏览按钮
    connect(ui->browseDownloadButton, &QPushButton::clicked, this, &MainWindow::browseDownloadPath);
    connect(ui->browseUploadButton, &QPushButton::clicked, this, &MainWindow::browseUploadFile);
    
    // HTTP操作按钮
    connect(ui->downloadButton, &QPushButton::clicked, this, &MainWindow::startDownload);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::startUpload);
    connect(ui->postButton, &QPushButton::clicked, this, &MainWindow::sendPostRequest);
    
    // 响应格式切换
    connect(ui->responseFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::formatResponse);
    
    // 菜单操作
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::clearData);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exitApplication);
}

void MainWindow::browseDownloadPath()
{
    QString filePath = QFileDialog::getSaveFileName(this, 
        tr("选择保存位置"), 
        QDir::homePath() + "/download_file", 
        tr("所有文件 (*.*)"));
    
    if (!filePath.isEmpty()) {
        ui->downloadPathEdit->setText(filePath);
    }
}

void MainWindow::browseUploadFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
        tr("选择要上传的文件"), 
        QDir::homePath(), 
        tr("所有文件 (*.*)"));
    
    if (!filePath.isEmpty()) {
        ui->uploadFileEdit->setText(filePath);
        uploadFilePath = filePath;
    }
}

void MainWindow::startDownload()
{
    QString url = ui->downloadUrlEdit->text().trimmed();
    QString savePath = ui->downloadPathEdit->text().trimmed();
    
    if (!validateUrl(url)) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入有效的下载URL"));
        return;
    }
    
    if (savePath.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("请选择文件保存路径"));
        return;
    }
    
    // 创建保存文件
    if (downloadFile) {
        downloadFile->close();
        delete downloadFile;
    }
    
    downloadFile = new QFile(savePath);
    if (!downloadFile->open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("文件错误"), tr("无法创建保存文件"));
        delete downloadFile;
        downloadFile = nullptr;
        return;
    }
    
    // 发起下载请求
    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("User-Agent", "Qt HTTP Client 1.0");
    
    currentDownloadReply = networkManager->get(request);
    
    connect(currentDownloadReply, &QNetworkReply::downloadProgress, 
            this, &MainWindow::downloadProgress);
    connect(currentDownloadReply, &QNetworkReply::finished, 
            this, &MainWindow::downloadFinished);
    connect(currentDownloadReply, &QNetworkReply::errorOccurred, 
            this, &MainWindow::networkError);
    connect(currentDownloadReply, &QNetworkReply::readyRead, [this]() {
        if (downloadFile) {
            downloadFile->write(currentDownloadReply->readAll());
        }
    });
    
    enableDownloadControls(false);
    ui->downloadProgressBar->setValue(0);
    updateStatus("正在下载...");
}

void MainWindow::startUpload()
{
    QString url = ui->uploadUrlEdit->text().trimmed();
    QString filePath = ui->uploadFileEdit->text().trimmed();
    
    if (!validateUrl(url)) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入有效的上传URL"));
        return;
    }
    
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        QMessageBox::warning(this, tr("文件错误"), tr("请选择有效的上传文件"));
        return;
    }
    
    // 创建multipart数据
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                      QVariant(QString("form-data; name=\"file\"; filename=\"%1\"")
                               .arg(QFileInfo(filePath).fileName())));
    
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("文件错误"), tr("无法读取上传文件"));
        delete file;
        delete multiPart;
        return;
    }
    
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);
    
    // 发起上传请求
    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("User-Agent", "Qt HTTP Client 1.0");
    
    currentUploadReply = networkManager->post(request, multiPart);
    multiPart->setParent(currentUploadReply);
    
    connect(currentUploadReply, &QNetworkReply::uploadProgress, 
            this, &MainWindow::uploadProgress);
    connect(currentUploadReply, &QNetworkReply::finished, 
            this, &MainWindow::uploadFinished);
    connect(currentUploadReply, &QNetworkReply::errorOccurred, 
            this, &MainWindow::networkError);
    
    enableUploadControls(false);
    ui->uploadProgressBar->setValue(0);
    updateStatus("正在上传...");
}

void MainWindow::sendPostRequest()
{
    QString url = ui->postUrlEdit->text().trimmed();
    QString data = ui->postDataEdit->toPlainText();
    QString contentType = ui->contentTypeCombo->currentText();
    
    if (!validateUrl(url)) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入有效的请求URL"));
        return;
    }
    
    if (data.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入请求数据"));
        return;
    }
    
    // 验证数据格式
    if (contentType.contains("json")) {
        QJsonParseError error;
        QJsonDocument::fromJson(data.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            QMessageBox::warning(this, tr("数据格式错误"), 
                                tr("JSON格式不正确: %1").arg(error.errorString()));
            return;
        }
    }
    
    // 发起POST请求
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    request.setRawHeader("User-Agent", "Qt HTTP Client 1.0");
    
    currentPostReply = networkManager->post(request, data.toUtf8());
    
    connect(currentPostReply, &QNetworkReply::finished, 
            this, &MainWindow::postRequestFinished);
    connect(currentPostReply, &QNetworkReply::errorOccurred, 
            this, &MainWindow::networkError);
    
    enablePostControls(false);
    updateStatus("正在发送POST请求...");
}

void MainWindow::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = (int)((bytesReceived * 100) / bytesTotal);
        ui->downloadProgressBar->setValue(progress);
        updateStatus(tr("下载进度: %1/%2 字节 (%3%)")
                    .arg(bytesReceived).arg(bytesTotal).arg(progress));
    }
}

void MainWindow::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = (int)((bytesSent * 100) / bytesTotal);
        ui->uploadProgressBar->setValue(progress);
        updateStatus(tr("上传进度: %1/%2 字节 (%3%)")
                    .arg(bytesSent).arg(bytesTotal).arg(progress));
    }
}

void MainWindow::downloadFinished()
{
    if (!currentDownloadReply) return;
    
    if (downloadFile) {
        downloadFile->close();
        delete downloadFile;
        downloadFile = nullptr;
    }
    
    if (currentDownloadReply->error() == QNetworkReply::NoError) {
        ui->downloadProgressBar->setValue(100);
        updateStatus("下载完成");
        QMessageBox::information(this, tr("下载完成"), tr("文件下载成功!"));
    } else {
        updateStatus("下载失败");
    }
    
    currentDownloadReply->deleteLater();
    currentDownloadReply = nullptr;
    enableDownloadControls(true);
}

void MainWindow::uploadFinished()
{
    if (!currentUploadReply) return;
    
    if (currentUploadReply->error() == QNetworkReply::NoError) {
        ui->uploadProgressBar->setValue(100);
        updateStatus("上传完成");
        
        // 显示响应数据
        QByteArray responseData = currentUploadReply->readAll();
        ui->responseDataEdit->setPlainText(QString::fromUtf8(responseData));
        ui->responseStatusValue->setText(QString::number(currentUploadReply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt()));
        ui->tabWidget->setCurrentIndex(3); // 切换到响应标签
        
        QMessageBox::information(this, tr("上传完成"), tr("文件上传成功!"));
    } else {
        updateStatus("上传失败");
    }
    
    currentUploadReply->deleteLater();
    currentUploadReply = nullptr;
    enableUploadControls(true);
}

void MainWindow::postRequestFinished()
{
    if (!currentPostReply) return;
    
    if (currentPostReply->error() == QNetworkReply::NoError) {
        updateStatus("POST请求完成");
        
        // 显示响应数据
        QByteArray responseData = currentPostReply->readAll();
        ui->responseDataEdit->setPlainText(QString::fromUtf8(responseData));
        ui->responseStatusValue->setText(QString::number(currentPostReply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt()));
        ui->tabWidget->setCurrentIndex(3); // 切换到响应标签
        
        // 自动格式化响应
        formatResponse();
        
        QMessageBox::information(this, tr("请求完成"), tr("POST请求发送成功!"));
    } else {
        updateStatus("POST请求失败");
    }
    
    currentPostReply->deleteLater();
    currentPostReply = nullptr;
    enablePostControls(true);
}

void MainWindow::networkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        QString operation = "网络操作";
        if (reply == currentDownloadReply) {
            operation = "下载";
            enableDownloadControls(true);
        } else if (reply == currentUploadReply) {
            operation = "上传";
            enableUploadControls(true);
        } else if (reply == currentPostReply) {
            operation = "POST请求";
            enablePostControls(true);
        }
        
        showNetworkError(operation, error);
        updateStatus(operation + "失败");
    }
}

void MainWindow::formatResponse()
{
    QString originalText = ui->responseDataEdit->toPlainText();
    if (originalText.isEmpty()) return;
    
    int formatType = ui->responseFormatCombo->currentIndex();
    QString formattedText;
    
    switch (formatType) {
    case 0: // 原始数据
        formattedText = originalText;
        break;
    case 1: // 格式化JSON
        formattedText = formatJsonString(originalText);
        break;
    case 2: // 格式化XML
        formattedText = formatXmlString(originalText);
        break;
    }
    
    ui->responseDataEdit->setPlainText(formattedText);
}

void MainWindow::clearData()
{
    ui->downloadUrlEdit->clear();
    ui->downloadPathEdit->clear();
    ui->uploadUrlEdit->clear();
    ui->uploadFileEdit->clear();
    ui->postUrlEdit->clear();
    ui->postDataEdit->clear();
    ui->responseDataEdit->clear();
    ui->responseStatusValue->setText("-");
    resetProgressBars();
    updateStatus("数据已清空");
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("关于"), 
        tr("HTTP客户端工具 v1.0\n\n"
           "功能特性:\n"
           "• 文件下载/上传\n"
           "• 进度实时显示\n"
           "• POST请求支持\n"
           "• JSON/XML数据格式化\n"
           "• 美观的现代化界面\n\n"
           "基于Qt框架开发"));
}

void MainWindow::exitApplication()
{
    close();
}

void MainWindow::updateStatus(const QString &message)
{
    ui->statusbar->showMessage(message);
}

QString MainWindow::formatJsonString(const QString &jsonString)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return jsonString; // 返回原始字符串如果不是有效JSON
    }
    
    return doc.toJson(QJsonDocument::Indented);
}

QString MainWindow::formatXmlString(const QString &xmlString)
{
    QString formattedXml;
    QXmlStreamReader reader(xmlString);
    QTextStream stream(&formattedXml);
    
    int indent = 0;
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isStartElement()) {
            stream << QString("  ").repeated(indent) << "<" << reader.name();
            
            // 添加属性
            auto attributes = reader.attributes();
            for (const auto &attr : attributes) {
                stream << " " << attr.name() << "=\"" << attr.value() << "\"";
            }
            stream << ">\n";
            indent++;
        } else if (reader.isEndElement()) {
            indent--;
            stream << QString("  ").repeated(indent) << "</" << reader.name() << ">\n";
        } else if (reader.isCharacters() && !reader.isWhitespace()) {
            stream << QString("  ").repeated(indent) << reader.text() << "\n";
        }
    }
    
    if (reader.hasError()) {
        return xmlString; // 返回原始字符串如果XML解析失败
    }
    
    return formattedXml;
}

void MainWindow::showNetworkError(const QString &operation, QNetworkReply::NetworkError error)
{
    QString errorMessage;
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorMessage = tr("连接被拒绝");
        break;
    case QNetworkReply::RemoteHostClosedError:
        errorMessage = tr("远程主机关闭连接");
        break;
    case QNetworkReply::HostNotFoundError:
        errorMessage = tr("主机未找到");
        break;
    case QNetworkReply::TimeoutError:
        errorMessage = tr("连接超时");
        break;
    case QNetworkReply::OperationCanceledError:
        errorMessage = tr("操作已取消");
        break;
    case QNetworkReply::SslHandshakeFailedError:
        errorMessage = tr("SSL握手失败");
        break;
    case QNetworkReply::TemporaryNetworkFailureError:
        errorMessage = tr("临时网络故障");
        break;
    case QNetworkReply::NetworkSessionFailedError:
        errorMessage = tr("网络会话失败");
        break;
    case QNetworkReply::BackgroundRequestNotAllowedError:
        errorMessage = tr("后台请求不被允许");
        break;
    case QNetworkReply::TooManyRedirectsError:
        errorMessage = tr("重定向次数过多");
        break;
    case QNetworkReply::InsecureRedirectError:
        errorMessage = tr("不安全的重定向");
        break;
    case QNetworkReply::ProxyConnectionRefusedError:
        errorMessage = tr("代理连接被拒绝");
        break;
    case QNetworkReply::ProxyConnectionClosedError:
        errorMessage = tr("代理连接已关闭");
        break;
    case QNetworkReply::ProxyNotFoundError:
        errorMessage = tr("代理未找到");
        break;
    case QNetworkReply::ProxyTimeoutError:
        errorMessage = tr("代理超时");
        break;
    case QNetworkReply::ProxyAuthenticationRequiredError:
        errorMessage = tr("代理需要身份验证");
        break;
    case QNetworkReply::ContentAccessDenied:
        errorMessage = tr("内容访问被拒绝");
        break;
    case QNetworkReply::ContentOperationNotPermittedError:
        errorMessage = tr("内容操作不被允许");
        break;
    case QNetworkReply::ContentNotFoundError:
        errorMessage = tr("内容未找到");
        break;
    case QNetworkReply::AuthenticationRequiredError:
        errorMessage = tr("需要身份验证");
        break;
    case QNetworkReply::ContentReSendError:
        errorMessage = tr("内容重发错误");
        break;
    case QNetworkReply::ContentConflictError:
        errorMessage = tr("内容冲突");
        break;
    case QNetworkReply::ContentGoneError:
        errorMessage = tr("内容已消失");
        break;
    case QNetworkReply::InternalServerError:
        errorMessage = tr("服务器内部错误");
        break;
    case QNetworkReply::OperationNotImplementedError:
        errorMessage = tr("操作未实现");
        break;
    case QNetworkReply::ServiceUnavailableError:
        errorMessage = tr("服务不可用");
        break;
    case QNetworkReply::ProtocolUnknownError:
        errorMessage = tr("未知协议");
        break;
    case QNetworkReply::ProtocolInvalidOperationError:
        errorMessage = tr("协议操作无效");
        break;
    case QNetworkReply::UnknownNetworkError:
        errorMessage = tr("未知网络错误");
        break;
    case QNetworkReply::UnknownProxyError:
        errorMessage = tr("未知代理错误");
        break;
    case QNetworkReply::UnknownContentError:
        errorMessage = tr("未知内容错误");
        break;
    case QNetworkReply::ProtocolFailure:
        errorMessage = tr("协议失败");
        break;
    case QNetworkReply::UnknownServerError:
        errorMessage = tr("未知服务器错误");
        break;
    default:
        errorMessage = tr("未知错误 (代码: %1)").arg(error);
        break;
    }
    
    QMessageBox::critical(this, tr("%1错误").arg(operation), 
                         tr("%1失败: %2").arg(operation, errorMessage));
}

void MainWindow::resetProgressBars()
{
    ui->downloadProgressBar->setValue(0);
    ui->uploadProgressBar->setValue(0);
}

bool MainWindow::validateUrl(const QString &url)
{
    QUrl qurl(url);
    return qurl.isValid() && !qurl.scheme().isEmpty() && !qurl.host().isEmpty();
}

void MainWindow::enableDownloadControls(bool enabled)
{
    ui->downloadUrlEdit->setEnabled(enabled);
    ui->downloadPathEdit->setEnabled(enabled);
    ui->browseDownloadButton->setEnabled(enabled);
    ui->downloadButton->setEnabled(enabled);
}

void MainWindow::enableUploadControls(bool enabled)
{
    ui->uploadUrlEdit->setEnabled(enabled);
    ui->uploadFileEdit->setEnabled(enabled);
    ui->browseUploadButton->setEnabled(enabled);
    ui->uploadButton->setEnabled(enabled);
}

void MainWindow::enablePostControls(bool enabled)
{
    ui->postUrlEdit->setEnabled(enabled);
    ui->contentTypeCombo->setEnabled(enabled);
    ui->postDataEdit->setEnabled(enabled);
    ui->postButton->setEnabled(enabled);
}
