#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isConnected(false)
{
    ui->setupUi(this);
    
    // 初始化数据库设置
    setupDatabase();
    
    // 连接信号和槽
    // XML 模块
    connect(ui->loadXmlButton, &QPushButton::clicked, this, &MainWindow::loadXmlFile);
    connect(ui->saveXmlButton, &QPushButton::clicked, this, &MainWindow::saveXmlFile);
    connect(ui->createSampleXmlButton, &QPushButton::clicked, this, &MainWindow::createSampleXml);
    connect(ui->xmlTreeWidget, &QTreeWidget::itemChanged, this, &MainWindow::onXmlTreeItemChanged);
    connect(ui->xmlTextEdit, &QTextEdit::textChanged, this, &MainWindow::onXmlTextChanged);
    
    // JSON 模块
    connect(ui->loadJsonButton, &QPushButton::clicked, this, &MainWindow::loadJsonFile);
    connect(ui->saveJsonButton, &QPushButton::clicked, this, &MainWindow::saveJsonFile);
    connect(ui->formatJsonButton, &QPushButton::clicked, this, &MainWindow::formatJson);
    connect(ui->validateJsonButton, &QPushButton::clicked, this, &MainWindow::validateJson);
    connect(ui->jsonTextEdit, &QTextEdit::textChanged, this, &MainWindow::onJsonTextChanged);
    
    // MySQL 模块
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::connectToDatabase);
    connect(ui->disconnectButton, &QPushButton::clicked, this, &MainWindow::disconnectFromDatabase);
    connect(ui->executeButton, &QPushButton::clicked, this, &MainWindow::executeQuery);
    connect(ui->clearQueryButton, &QPushButton::clicked, this, &MainWindow::clearQuery);
    
    // 设置初始状态
    ui->xmlTreeWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->jsonTreeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // 状态栏初始信息
    ui->statusbar->showMessage("欢迎使用多功能文件处理工具");
}

MainWindow::~MainWindow()
{
    if (isConnected) {
        database.close();
    }
    delete ui;
}

// ==================== XML 功能实现 ====================

void MainWindow::loadXmlFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "打开XML文件", QString(), "XML Files (*.xml)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString errorMsg;
            int errorLine, errorColumn;
            
            if (xmlDocument.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
                currentXmlFilePath = fileName;
                ui->xmlTreeWidget->clear();
                populateXmlTree(xmlDocument.documentElement());
                updateXmlText();
                ui->statusbar->showMessage(QString("已加载XML文件: %1").arg(fileName));
            } else {
                QMessageBox::warning(this, "错误", 
                    QString("解析XML文件失败:\n%1\n行: %2, 列: %3")
                    .arg(errorMsg).arg(errorLine).arg(errorColumn));
            }
            file.close();
        } else {
            QMessageBox::warning(this, "错误", "无法打开文件: " + fileName);
        }
    }
}

void MainWindow::saveXmlFile()
{
    if (xmlDocument.isNull()) {
        QMessageBox::information(this, "提示", "没有XML内容可保存");
        return;
    }
    
    QString fileName = currentXmlFilePath;
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(this, 
            "保存XML文件", QString(), "XML Files (*.xml)");
    }
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << xmlDocument.toString(2);
            file.close();
            currentXmlFilePath = fileName;
            ui->statusbar->showMessage(QString("XML文件已保存: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, "错误", "无法保存文件: " + fileName);
        }
    }
}

void MainWindow::createSampleXml()
{
    QString sampleXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<library>
    <book id="1">
        <title>C++ 程序设计</title>
        <author>张三</author>
        <price>89.50</price>
        <category>编程</category>
    </book>
    <book id="2">
        <title>Qt 应用开发</title>
        <author>李四</author>
        <price>99.00</price>
        <category>软件开发</category>
    </book>
    <magazine id="101">
        <title>程序员杂志</title>
        <issue>2024年第1期</issue>
        <price>15.00</price>
    </magazine>
</library>)";

    QString errorMsg;
    int errorLine, errorColumn;
    
    if (xmlDocument.setContent(sampleXml, &errorMsg, &errorLine, &errorColumn)) {
        ui->xmlTreeWidget->clear();
        populateXmlTree(xmlDocument.documentElement());
        updateXmlText();
        currentXmlFilePath.clear();
        ui->statusbar->showMessage("已创建示例XML");
    }
}

void MainWindow::populateXmlTree(const QDomElement &element, QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *item;
    if (parentItem) {
        item = new QTreeWidgetItem(parentItem);
    } else {
        item = new QTreeWidgetItem(ui->xmlTreeWidget);
    }
    
    item->setText(0, element.tagName());
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    
    // 添加属性
    QDomNamedNodeMap attributes = element.attributes();
    for (int i = 0; i < attributes.count(); ++i) {
        QDomNode attr = attributes.item(i);
        QTreeWidgetItem *attrItem = new QTreeWidgetItem(item);
        attrItem->setText(0, QString("@%1").arg(attr.nodeName()));
        attrItem->setText(1, attr.nodeValue());
        attrItem->setFlags(attrItem->flags() | Qt::ItemIsEditable);
    }
    
    // 处理子节点
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        if (child.isElement()) {
            populateXmlTree(child.toElement(), item);
        } else if (child.isText()) {
            QString text = child.nodeValue().trimmed();
            if (!text.isEmpty()) {
                item->setText(1, text);
            }
        }
        child = child.nextSibling();
    }
    
    item->setExpanded(true);
}

void MainWindow::onXmlTreeItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == 1) {  // 值被修改
        updateXmlFromTree();
        updateXmlText();
    }
}

void MainWindow::updateXmlFromTree()
{
    // 这里可以实现从树形视图更新DOM文档的逻辑
    // 为简化实现，这里暂时不实现反向更新
}

void MainWindow::updateXmlText()
{
    if (!xmlDocument.isNull()) {
        ui->xmlTextEdit->blockSignals(true);
        ui->xmlTextEdit->setPlainText(xmlDocument.toString(2));
        ui->xmlTextEdit->blockSignals(false);
    }
}

void MainWindow::onXmlTextChanged()
{
    QString text = ui->xmlTextEdit->toPlainText();
    QDomDocument newDoc;
    QString errorMsg;
    int errorLine, errorColumn;
    
    if (newDoc.setContent(text, &errorMsg, &errorLine, &errorColumn)) {
        xmlDocument = newDoc;
        ui->xmlTreeWidget->blockSignals(true);
        ui->xmlTreeWidget->clear();
        populateXmlTree(xmlDocument.documentElement());
        ui->xmlTreeWidget->blockSignals(false);
    }
}

// ==================== JSON 功能实现 ====================

void MainWindow::loadJsonFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "打开JSON文件", QString(), "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            QJsonParseError error;
            jsonDocument = QJsonDocument::fromJson(data, &error);
            
            if (error.error == QJsonParseError::NoError) {
                currentJsonFilePath = fileName;
                ui->jsonTreeWidget->clear();
                populateJsonTree(jsonDocument.object().isEmpty() ? 
                    QJsonValue(jsonDocument.array()) : QJsonValue(jsonDocument.object()));
                updateJsonText();
                ui->statusbar->showMessage(QString("已加载JSON文件: %1").arg(fileName));
            } else {
                QMessageBox::warning(this, "错误", 
                    QString("解析JSON文件失败:\n%1").arg(error.errorString()));
            }
            file.close();
        } else {
            QMessageBox::warning(this, "错误", "无法打开文件: " + fileName);
        }
    }
}

void MainWindow::saveJsonFile()
{
    if (jsonDocument.isNull()) {
        QMessageBox::information(this, "提示", "没有JSON内容可保存");
        return;
    }
    
    QString fileName = currentJsonFilePath;
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(this, 
            "保存JSON文件", QString(), "JSON Files (*.json)");
    }
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(jsonDocument.toJson());
            file.close();
            currentJsonFilePath = fileName;
            ui->statusbar->showMessage(QString("JSON文件已保存: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, "错误", "无法保存文件: " + fileName);
        }
    }
}

void MainWindow::formatJson()
{
    QString text = ui->jsonTextEdit->toPlainText();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
    
    if (error.error == QJsonParseError::NoError) {
        jsonDocument = doc;
        updateJsonText();
        ui->jsonTreeWidget->clear();
        populateJsonTree(doc.object().isEmpty() ? 
            QJsonValue(doc.array()) : QJsonValue(doc.object()));
        ui->statusbar->showMessage("JSON格式化完成");
    } else {
        QMessageBox::warning(this, "错误", 
            QString("JSON格式错误:\n%1").arg(error.errorString()));
    }
}

void MainWindow::validateJson()
{
    QString text = ui->jsonTextEdit->toPlainText();
    QJsonParseError error;
    QJsonDocument::fromJson(text.toUtf8(), &error);
    
    if (error.error == QJsonParseError::NoError) {
        QMessageBox::information(this, "验证结果", "JSON格式正确！");
    } else {
        QMessageBox::warning(this, "验证结果", 
            QString("JSON格式错误:\n%1\n位置: %2").arg(error.errorString()).arg(error.offset));
    }
}

void MainWindow::populateJsonTree(const QJsonValue &value, QTreeWidgetItem *parentItem, const QString &key)
{
    QTreeWidgetItem *item;
    if (parentItem) {
        item = new QTreeWidgetItem(parentItem);
    } else {
        item = new QTreeWidgetItem(ui->jsonTreeWidget);
    }
    
    item->setText(0, key.isEmpty() ? "root" : key);
    
    switch (value.type()) {
        case QJsonValue::Object: {
            item->setText(1, "");
            item->setText(2, "Object");
            QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                populateJsonTree(it.value(), item, it.key());
            }
            break;
        }
        case QJsonValue::Array: {
            item->setText(1, "");
            item->setText(2, "Array");
            QJsonArray array = value.toArray();
            for (int i = 0; i < array.size(); ++i) {
                populateJsonTree(array[i], item, QString("[%1]").arg(i));
            }
            break;
        }
        case QJsonValue::String:
            item->setText(1, value.toString());
            item->setText(2, "String");
            break;
        case QJsonValue::Double:
            item->setText(1, QString::number(value.toDouble()));
            item->setText(2, "Number");
            break;
        case QJsonValue::Bool:
            item->setText(1, value.toBool() ? "true" : "false");
            item->setText(2, "Boolean");
            break;
        case QJsonValue::Null:
            item->setText(1, "null");
            item->setText(2, "Null");
            break;
        default:
            item->setText(1, "undefined");
            item->setText(2, "Undefined");
            break;
    }
    
    item->setExpanded(true);
}

void MainWindow::updateJsonText()
{
    if (!jsonDocument.isNull()) {
        ui->jsonTextEdit->blockSignals(true);
        ui->jsonTextEdit->setPlainText(jsonDocument.toJson());
        ui->jsonTextEdit->blockSignals(false);
    }
}

void MainWindow::onJsonTextChanged()
{
    QString text = ui->jsonTextEdit->toPlainText();
    QJsonParseError error;
    QJsonDocument newDoc = QJsonDocument::fromJson(text.toUtf8(), &error);
    
    if (error.error == QJsonParseError::NoError) {
        jsonDocument = newDoc;
        ui->jsonTreeWidget->blockSignals(true);
        ui->jsonTreeWidget->clear();
        populateJsonTree(newDoc.object().isEmpty() ? 
            QJsonValue(newDoc.array()) : QJsonValue(newDoc.object()));
        ui->jsonTreeWidget->blockSignals(false);
    }
}

// ==================== MySQL 功能实现 ====================

void MainWindow::setupDatabase()
{
    database = QSqlDatabase::addDatabase("QMYSQL");
}

void MainWindow::connectToDatabase()
{
    if (isConnected) {
        QMessageBox::information(this, "提示", "数据库已连接");
        return;
    }
    
    database.setHostName(ui->hostEdit->text());
    database.setPort(ui->portEdit->text().toInt());
    database.setDatabaseName(ui->databaseEdit->text());
    database.setUserName(ui->usernameEdit->text());
    database.setPassword(ui->passwordEdit->text());
    
    if (database.open()) {
        isConnected = true;
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->executeButton->setEnabled(true);
        
        logMessage("数据库连接成功！");
        ui->statusbar->showMessage("数据库已连接");
        
        // 显示数据库信息
        QSqlQuery query("SELECT VERSION() as version");
        if (query.exec() && query.next()) {
            logMessage(QString("MySQL版本: %1").arg(query.value("version").toString()));
        }
    } else {
        QMessageBox::critical(this, "连接错误", 
            QString("无法连接到数据库:\n%1").arg(database.lastError().text()));
        logMessage("数据库连接失败: " + database.lastError().text());
    }
}

void MainWindow::disconnectFromDatabase()
{
    if (isConnected) {
        database.close();
        isConnected = false;
        ui->connectButton->setEnabled(true);
        ui->disconnectButton->setEnabled(false);
        ui->executeButton->setEnabled(false);
        ui->resultTableWidget->clear();
        ui->resultTableWidget->setRowCount(0);
        ui->resultTableWidget->setColumnCount(0);
        
        logMessage("数据库连接已断开");
        ui->statusbar->showMessage("数据库连接已断开");
    }
}

void MainWindow::executeQuery()
{
    if (!isConnected) {
        QMessageBox::warning(this, "错误", "请先连接数据库");
        return;
    }
    
    QString sql = ui->sqlTextEdit->toPlainText().trimmed();
    if (sql.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入SQL查询语句");
        return;
    }
    
    QSqlQuery query;
    if (query.exec(sql)) {
        if (query.isSelect()) {
            displayQueryResults(query);
            logMessage(QString("查询执行成功，返回 %1 行数据").arg(query.size()));
        } else {
            int affectedRows = query.numRowsAffected();
            logMessage(QString("查询执行成功，影响 %1 行").arg(affectedRows));
            ui->resultTableWidget->clear();
            ui->resultTableWidget->setRowCount(0);
            ui->resultTableWidget->setColumnCount(0);
        }
    } else {
        QString error = query.lastError().text();
        QMessageBox::critical(this, "查询错误", error);
        logMessage("查询执行失败: " + error);
    }
}

void MainWindow::clearQuery()
{
    ui->sqlTextEdit->clear();
    ui->resultTableWidget->clear();
    ui->resultTableWidget->setRowCount(0);
    ui->resultTableWidget->setColumnCount(0);
    ui->messageTextEdit->clear();
}

void MainWindow::displayQueryResults(const QSqlQuery &query)
{
    QSqlRecord record = query.record();
    int columnCount = record.count();
    
    ui->resultTableWidget->setColumnCount(columnCount);
    
    // 设置列标题
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers << record.fieldName(i);
    }
    ui->resultTableWidget->setHorizontalHeaderLabels(headers);
    
    // 填充数据
    QSqlQuery tempQuery = query;  // 创建副本以便重新遍历
    tempQuery.first();
    tempQuery.previous();  // 回到开始位置
    
    QList<QStringList> rows;
    while (tempQuery.next()) {
        QStringList row;
        for (int i = 0; i < columnCount; ++i) {
            row << tempQuery.value(i).toString();
        }
        rows << row;
    }
    
    ui->resultTableWidget->setRowCount(rows.size());
    
    for (int row = 0; row < rows.size(); ++row) {
        for (int col = 0; col < columnCount; ++col) {
            ui->resultTableWidget->setItem(row, col, 
                new QTableWidgetItem(rows[row][col]));
        }
    }
    
    // 调整列宽
    ui->resultTableWidget->resizeColumnsToContents();
}

void MainWindow::logMessage(const QString &message)
{
    ui->messageTextEdit->append(QString("[%1] %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(message));
}
