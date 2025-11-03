#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>

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
    void onDigitClicked();
    void onOperatorClicked();
    void onClearClicked();
    void onEqualClicked();
    void onMemoryClicked();

private:
    void setupUI();
    void setupTitleBar();
    void setupDisplayArea();
    void setupButtonArea();
    QWidget* createTitleBarWidget();
    QWidget* createButtonAreaWidget();
    QPushButton* createButton(const QString &text, int row, int col, 
                     const QString &buttonName, const QString &styleClass = "");
    void connectSignals();
    
    // 计算逻辑方法
    double performCalculation(double operand1, double operand2, const QString &operation);
    void updateDisplay(double value);
    void resetCalculator();

    Ui::MainWindow *ui;
    
    // 显示区域
    QLabel *displayLabel;
    
    // 标题栏控件
    QPushButton *menuButton;
    QLabel *titleLabel;
    QPushButton *historyButton;
    QPushButton *minimizeButton;
    QPushButton *maximizeButton;
    QPushButton *closeButton;
    
    // 按钮网格
    QGridLayout *buttonGrid;
    QWidget *centralWidget;
    
    // 存储所有按钮的引用
    QPushButton *btnMC, *btnMR, *btnMPlus, *btnMS;
    QPushButton *btnPercent, *btnSqrt, *btnSquare, *btnReciprocal;
    QPushButton *btnCE, *btnC, *btnBackspace, *btnDivide;
    QPushButton *btn7, *btn8, *btn9, *btnMultiply;
    QPushButton *btn4, *btn5, *btn6, *btnMinus;
    QPushButton *btn1, *btn2, *btn3, *btnPlus;
    QPushButton *btnPlusMinus, *btn0, *btnDecimal, *btnEqual;
    
    // 计算器状态变量
    double currentValue;
    double storedValue;
    QString currentOperation;
    bool waitingForOperand;
    bool justCalculated;
    double memoryValue;
};

#endif // MAINWINDOW_H
