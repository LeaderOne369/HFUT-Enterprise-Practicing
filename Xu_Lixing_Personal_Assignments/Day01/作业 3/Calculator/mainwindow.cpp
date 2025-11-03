#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QSizePolicy>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentValue(0), storedValue(0), 
      waitingForOperand(true), justCalculated(false), memoryValue(0)
{
    setupUI();
    
    // 设置窗口属性
    setWindowTitle("计算器");
    setFixedSize(320, 500);
    
    // 设置窗口标志以隐藏默认标题栏
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    
    // 设置样式表
    QString styleSheet = R"(
        MainWindow {
            background-color: #f3f3f3;
        }
        
        #titleBar {
            background-color: #f3f3f3;
            border: none;
        }
        
        #menuButton {
            background-color: transparent;
            border: none;
            font-size: 16px;
            padding: 8px;
        }
        
        #menuButton:hover {
            background-color: #e5e5e5;
        }
        
        #titleLabel {
            background-color: transparent;
            border: none;
            font-size: 20px;
            font-weight: bold;
            color: #000000;
        }
        
        #historyButton, #minimizeButton, #maximizeButton, #closeButton {
            background-color: transparent;
            border: none;
            font-size: 12px;
            padding: 8px;
            min-width: 46px;
            min-height: 32px;
        }
        
        #historyButton:hover, #minimizeButton:hover, #maximizeButton:hover {
            background-color: #e5e5e5;
        }
        
        #closeButton:hover {
            background-color: #e81123;
            color: white;
        }
        
        #displayLabel {
            background-color: #f3f3f3;
            border: none;
            font-size: 48px;
            font-weight: bold;
            color: #000000;
            padding: 10px;
        }
        
        QPushButton {
            background-color: #fafafa;
            border: 1px solid #e1e1e1;
            border-radius: 4px;
            font-size: 16px;
            font-weight: 600;
            min-width: 60px;
            min-height: 60px;
        }
        
        QPushButton:hover {
            background-color: #e5f3ff;
        }
        
        QPushButton:pressed {
            background-color: #cce4f7;
        }
        
        .numberButton {
            background-color: #fafafa;
        }
        
        .operatorButton {
            background-color: #fafafa;
        }
        
        .functionButton {
            background-color: #fafafa;
        }
        
        .memoryButton {
            background-color: #fafafa;
            font-size: 14px;
        }
        
        .equalButton {
            background-color: #005a9e;
            color: white;
            font-size: 18px;
        }
        
        .equalButton:hover {
            background-color: #106ebe;
        }
        
        .disabledButton {
            background-color: #f3f3f3;
            color: #8a8a8a;
            border: 1px solid #e1e1e1;
        }
        
        .disabledButton:hover {
            background-color: #f3f3f3;
        }
    )";
    
    setStyleSheet(styleSheet);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 设置标题栏
    setupTitleBar();
    
    // 设置显示区域
    setupDisplayArea();
    
    // 设置按钮区域
    setupButtonArea();
    
    mainLayout->addWidget(createTitleBarWidget());
    mainLayout->addWidget(displayLabel);
    mainLayout->addWidget(createButtonAreaWidget());
}

QWidget* MainWindow::createTitleBarWidget()
{
    QWidget *titleBarWidget = new QWidget();
    titleBarWidget->setObjectName("titleBar");
    titleBarWidget->setFixedHeight(48);
    
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBarWidget);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    
    // 菜单按钮
    menuButton = new QPushButton("☰", titleBarWidget);
    menuButton->setObjectName("menuButton");
    
    // 标题
    titleLabel = new QLabel("标准", titleBarWidget);
    titleLabel->setObjectName("titleLabel");
    
    // 弹性空间
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    // 历史记录按钮
    historyButton = new QPushButton("⟲", titleBarWidget);
    historyButton->setObjectName("historyButton");
    
    // 窗口控制按钮
    minimizeButton = new QPushButton("─", titleBarWidget);
    minimizeButton->setObjectName("minimizeButton");
    
    maximizeButton = new QPushButton("□", titleBarWidget);
    maximizeButton->setObjectName("maximizeButton");
    
    closeButton = new QPushButton("✕", titleBarWidget);
    closeButton->setObjectName("closeButton");
    
    titleLayout->addWidget(menuButton);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(spacer);
    titleLayout->addWidget(historyButton);
    titleLayout->addWidget(minimizeButton);
    titleLayout->addWidget(maximizeButton);
    titleLayout->addWidget(closeButton);
    
    // 连接菜单按钮信号
    connect(menuButton, &QPushButton::clicked, [this]() {
        // 创建菜单
        QMenu *menu = new QMenu(this);
        
        QAction *standardAction = menu->addAction("标准");
        standardAction->setCheckable(true);
        standardAction->setChecked(true);
        
        menu->addSeparator();
        QAction *aboutAction = menu->addAction("关于");
        
        // 连接菜单项的信号
        connect(standardAction, &QAction::triggered, [this]() {
            titleLabel->setText("标准");
            // 当前就是标准模式
        });
        
        connect(aboutAction, &QAction::triggered, [this]() {
            QMessageBox::about(this, "关于", 
                "Qt 计算器 v1.0\n\n"
                "高度还原 Windows 10/11 标准计算器\n"
                "支持基础四则运算、高级数学函数和内存操作\n\n"
                "布局：4×7 按钮网格，统一美观\n"
                "开发工具：Qt Framework\n"
                "界面设计：Modern Flat UI");
        });
        
        // 显示菜单
        menu->exec(menuButton->mapToGlobal(QPoint(0, menuButton->height())));
        menu->deleteLater();
    });
    
    // 连接历史记录按钮信号
    connect(historyButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "历史记录", 
            "历史记录功能\n\n"
            "这里将显示最近的计算历史。\n"
            "当前版本暂未实现此功能，\n"
            "可在后续版本中添加。");
    });
    
    // 连接窗口控制信号
    connect(minimizeButton, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(maximizeButton, &QPushButton::clicked, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    
    return titleBarWidget;
}

void MainWindow::setupTitleBar()
{
    // 这个方法在 createTitleBarWidget 中实现
}

void MainWindow::setupDisplayArea()
{
    displayLabel = new QLabel("0");
    displayLabel->setObjectName("displayLabel");
    displayLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    displayLabel->setFixedHeight(80);
}

void MainWindow::setupButtonArea()
{
    // 这个方法在 createButtonAreaWidget 中实现
}

QWidget* MainWindow::createButtonAreaWidget()
{
    QWidget *buttonAreaWidget = new QWidget();
    buttonGrid = new QGridLayout(buttonAreaWidget);
    buttonGrid->setSpacing(2);
    buttonGrid->setContentsMargins(5, 5, 5, 5);
    
    // 第一行：存储操作（4个按钮）
    btnMC = createButton("MC", 0, 0, "btnMC", "memoryButton");
    btnMR = createButton("MR", 0, 1, "btnMR", "memoryButton");
    btnMPlus = createButton("M+", 0, 2, "btnMPlus", "memoryButton");
    btnMS = createButton("MS", 0, 3, "btnMS", "memoryButton");
    
    // 第二行
    btnPercent = createButton("%", 1, 0, "btnPercent", "functionButton");
    btnSqrt = createButton("√", 1, 1, "btnSqrt", "functionButton");
    btnSquare = createButton("x²", 1, 2, "btnSquare", "functionButton");
    btnReciprocal = createButton("1/x", 1, 3, "btnReciprocal", "functionButton");
    
    // 第三行
    btnCE = createButton("CE", 2, 0, "btnCE", "functionButton");
    btnC = createButton("C", 2, 1, "btnC", "functionButton");
    btnBackspace = createButton("⌫", 2, 2, "btnBackspace", "functionButton");
    btnDivide = createButton("÷", 2, 3, "btnDivide", "operatorButton");
    
    // 第四行
    btn7 = createButton("7", 3, 0, "btn7", "numberButton");
    btn8 = createButton("8", 3, 1, "btn8", "numberButton");
    btn9 = createButton("9", 3, 2, "btn9", "numberButton");
    btnMultiply = createButton("×", 3, 3, "btnMultiply", "operatorButton");
    
    // 第五行
    btn4 = createButton("4", 4, 0, "btn4", "numberButton");
    btn5 = createButton("5", 4, 1, "btn5", "numberButton");
    btn6 = createButton("6", 4, 2, "btn6", "numberButton");
    btnMinus = createButton("−", 4, 3, "btnMinus", "operatorButton");
    
    // 第六行
    btn1 = createButton("1", 5, 0, "btn1", "numberButton");
    btn2 = createButton("2", 5, 1, "btn2", "numberButton");
    btn3 = createButton("3", 5, 2, "btn3", "numberButton");
    btnPlus = createButton("+", 5, 3, "btnPlus", "operatorButton");
    
    // 第七行
    btnPlusMinus = createButton("±", 6, 0, "btnPlusMinus", "functionButton");
    btn0 = createButton("0", 6, 1, "btn0", "numberButton");
    btnDecimal = createButton(".", 6, 2, "btnDecimal", "numberButton");
    btnEqual = createButton("=", 6, 3, "btnEqual", "equalButton");
    
    // 连接信号到槽
    connectSignals();
    
    return buttonAreaWidget;
}

QPushButton* MainWindow::createButton(const QString &text, int row, int col, 
                                     const QString &buttonName, const QString &styleClass)
{
    QPushButton *button = new QPushButton(text);
    button->setObjectName(buttonName);
    if (!styleClass.isEmpty()) {
        button->setProperty("class", styleClass);
    }
    buttonGrid->addWidget(button, row, col);
    return button;
}

void MainWindow::connectSignals()
{
    // 数字按钮
    connect(btn0, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn1, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn2, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn3, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn4, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn5, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn6, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn7, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn8, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btn9, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    connect(btnDecimal, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    
    // 运算符按钮
    connect(btnPlus, &QPushButton::clicked, this, &MainWindow::onOperatorClicked);
    connect(btnMinus, &QPushButton::clicked, this, &MainWindow::onOperatorClicked);
    connect(btnMultiply, &QPushButton::clicked, this, &MainWindow::onOperatorClicked);
    connect(btnDivide, &QPushButton::clicked, this, &MainWindow::onOperatorClicked);
    
    // 功能按钮
    connect(btnEqual, &QPushButton::clicked, this, &MainWindow::onEqualClicked);
    connect(btnC, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(btnCE, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    
    // 退格按钮
    connect(btnBackspace, &QPushButton::clicked, [this]() {
        QString current = displayLabel->text();
        if (current.length() > 1) {
            current.chop(1);
            displayLabel->setText(current);
            currentValue = current.toDouble();
        } else {
            displayLabel->setText("0");
            currentValue = 0;
            waitingForOperand = true;
        }
    });
    
    // 正负号按钮
    connect(btnPlusMinus, &QPushButton::clicked, [this]() {
        double value = displayLabel->text().toDouble();
        value = -value;
        updateDisplay(value);
        currentValue = value;
    });
    
    // 高级功能按钮
    connect(btnPercent, &QPushButton::clicked, [this]() {
        double value = displayLabel->text().toDouble();
        value = value / 100.0;
        updateDisplay(value);
        currentValue = value;
    });
    
    connect(btnSqrt, &QPushButton::clicked, [this]() {
        double value = displayLabel->text().toDouble();
        if (value >= 0) {
            value = sqrt(value);
            updateDisplay(value);
            currentValue = value;
        }
    });
    
    connect(btnSquare, &QPushButton::clicked, [this]() {
        double value = displayLabel->text().toDouble();
        value = value * value;
        updateDisplay(value);
        currentValue = value;
    });
    
    connect(btnReciprocal, &QPushButton::clicked, [this]() {
        double value = displayLabel->text().toDouble();
        if (value != 0) {
            value = 1.0 / value;
            updateDisplay(value);
            currentValue = value;
        }
    });
    
    // 存储按钮
    connect(btnMC, &QPushButton::clicked, this, &MainWindow::onMemoryClicked);
    connect(btnMR, &QPushButton::clicked, this, &MainWindow::onMemoryClicked);
    connect(btnMPlus, &QPushButton::clicked, this, &MainWindow::onMemoryClicked);
    connect(btnMS, &QPushButton::clicked, this, &MainWindow::onMemoryClicked);
}

// 槽函数实现（完整计算逻辑）
void MainWindow::onDigitClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QString digitText = button->text();
    
    if (justCalculated) {
        // 如果刚刚完成计算，重新开始
        resetCalculator();
        justCalculated = false;
    }
    
    if (displayLabel->text() == "0" && digitText != ".") {
        displayLabel->setText(digitText);
    } else {
        // 检查小数点重复
        if (digitText == "." && displayLabel->text().contains(".")) {
            return;
        }
        
        if (waitingForOperand) {
            displayLabel->setText(digitText);
            waitingForOperand = false;
        } else {
            displayLabel->setText(displayLabel->text() + digitText);
        }
    }
    
    currentValue = displayLabel->text().toDouble();
}

void MainWindow::onOperatorClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QString operatorText = button->text();
    double displayValue = displayLabel->text().toDouble();
    
    if (!currentOperation.isEmpty() && !waitingForOperand) {
        // 执行之前的操作
        double result = performCalculation(storedValue, displayValue, currentOperation);
        updateDisplay(result);
        currentValue = result;
    } else {
        currentValue = displayValue;
    }
    
    // 存储当前值和操作符
    storedValue = currentValue;
    currentOperation = operatorText;
    waitingForOperand = true;
    justCalculated = false;
}

void MainWindow::onClearClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    if (button->text() == "C") {
        // 全部清除
        resetCalculator();
    } else if (button->text() == "CE") {
        // 清除当前输入
        displayLabel->setText("0");
        currentValue = 0;
        waitingForOperand = true;
    }
}

void MainWindow::onEqualClicked()
{
    if (!currentOperation.isEmpty() && !waitingForOperand) {
        double displayValue = displayLabel->text().toDouble();
        double result = performCalculation(storedValue, displayValue, currentOperation);
        updateDisplay(result);
        
        currentValue = result;
        currentOperation.clear();
        waitingForOperand = true;
        justCalculated = true;
    }
}

void MainWindow::onMemoryClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QString buttonText = button->text();
    double displayValue = displayLabel->text().toDouble();
    
    if (buttonText == "MC") {
        // 内存清除
        memoryValue = 0;
    } else if (buttonText == "MR") {
        // 内存读取
        updateDisplay(memoryValue);
        currentValue = memoryValue;
        waitingForOperand = true;
    } else if (buttonText == "MS") {
        // 内存存储
        memoryValue = displayValue;
    } else if (buttonText == "M+") {
        // 内存加
        memoryValue += displayValue;
    }
}

// 新增的计算逻辑方法
double MainWindow::performCalculation(double operand1, double operand2, const QString &operation)
{
    if (operation == "+") {
        return operand1 + operand2;
    } else if (operation == "−" || operation == "-") {
        return operand1 - operand2;
    } else if (operation == "×" || operation == "*") {
        return operand1 * operand2;
    } else if (operation == "÷" || operation == "/") {
        if (operand2 != 0) {
            return operand1 / operand2;
        } else {
            // 除零错误
            return 0;
        }
    }
    return operand2;
}

void MainWindow::updateDisplay(double value)
{
    // 格式化显示值
    if (value == (long long)value) {
        // 整数显示
        displayLabel->setText(QString::number((long long)value));
    } else {
        // 小数显示，最多15位精度
        QString text = QString::number(value, 'g', 15);
        displayLabel->setText(text);
    }
}

void MainWindow::resetCalculator()
{
    currentValue = 0;
    storedValue = 0;
    currentOperation.clear();
    waitingForOperand = true;
    justCalculated = false;
    displayLabel->setText("0");
}
