#include "connectdialog.h"
#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    
    connect(ui->clientRadio, &QRadioButton::toggled, this, &ConnectDialog::onModeChanged);
    connect(ui->serverRadio, &QRadioButton::toggled, this, &ConnectDialog::onModeChanged);
    
    onModeChanged(); // 初始化界面状态
    applyIOSStyle(); // 应用iOS风格样式
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

QString ConnectDialog::getServerHost() const
{
    return ui->hostEdit->text().trimmed();
}

quint16 ConnectDialog::getServerPort() const
{
    return static_cast<quint16>(ui->portSpinBox->value());
}

QString ConnectDialog::getNickname() const
{
    return ui->nicknameEdit->text().trimmed();
}

bool ConnectDialog::isServerMode() const
{
    return ui->serverRadio->isChecked();
}

void ConnectDialog::onModeChanged()
{
    bool isClient = ui->clientRadio->isChecked();
    
    // 客户端模式需要服务器地址，服务器模式不需要
    ui->hostLabel->setVisible(isClient);
    ui->hostEdit->setVisible(isClient);
    
    if (isClient) {
        ui->connectionGroupBox->setTitle("连接设置");
        ui->portLabel->setText("服务器端口:");
    } else {
        ui->connectionGroupBox->setTitle("服务器设置");
        ui->portLabel->setText("监听端口:");
    }
}

void ConnectDialog::applyIOSStyle()
{
    // iOS风格的连接对话框样式
    QString iosDialogStyle = R"(
        QDialog {
            background-color: #f2f2f7;
            border-radius: 16px;
        }
        
        QGroupBox {
            font-size: 18px;
            font-weight: 600;
            color: #1c1c1e;
            border: 2px solid #e5e5ea;
            border-radius: 12px;
            margin-top: 16px;
            padding: 20px 20px 24px 20px;
            background-color: #ffffff;
        }
        
        QGroupBox#modeGroupBox {
            padding: 12px 20px 16px 20px;
        }
        
        QGroupBox#connectionGroupBox {
            padding: 20px 20px 24px 20px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #007aff;
            font-weight: 700;
        }
        
        QRadioButton {
            font-size: 16px;
            font-weight: 500;
            color: #1c1c1e;
            spacing: 8px;
            padding: 8px;
        }
        
        QRadioButton::indicator {
            width: 20px;
            height: 20px;
            border-radius: 10px;
        }
        
        QRadioButton::indicator:unchecked {
            background-color: #ffffff;
            border: 2px solid #c6c6c8;
        }
        
        QRadioButton::indicator:unchecked:hover {
            border: 2px solid #007aff;
        }
        
        QRadioButton::indicator:checked {
            background-color: #007aff;
            border: 2px solid #007aff;
        }
        
        QRadioButton::indicator:checked:after {
            content: "";
            width: 8px;
            height: 8px;
            border-radius: 4px;
            background-color: #ffffff;
            position: absolute;
            top: 6px;
            left: 6px;
        }
        
        QLabel {
            font-size: 16px;
            font-weight: 500;
            color: #1c1c1e;
            padding: 4px 0px;
        }
        
        QLineEdit {
            background-color: #f2f2f7;
            border: 2px solid #e5e5ea;
            border-radius: 12px;
            padding: 16px 20px;
            font-size: 16px;
            color: #1c1c1e;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
            min-height: 24px;
        }
        
        QLineEdit:focus {
            border-color: #007aff;
            background-color: #ffffff;
            outline: none;
        }
        
        QSpinBox {
            background-color: #f2f2f7;
            border: 2px solid #e5e5ea;
            border-radius: 12px;
            padding: 16px 20px;
            font-size: 16px;
            color: #1c1c1e;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
            min-height: 24px;
        }
        
        QSpinBox:focus {
            border-color: #007aff;
            background-color: #ffffff;
            outline: none;
        }
        
        QSpinBox::up-button,
        QSpinBox::down-button {
            background-color: transparent;
            border: none;
            width: 16px;
            height: 16px;
        }
        
        QSpinBox::up-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-bottom: 6px solid #007aff;
            width: 0px;
            height: 0px;
        }
        
        QSpinBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #007aff;
            width: 0px;
            height: 0px;
        }
        
        QDialogButtonBox {
            margin-top: 16px;
        }
        
        QDialogButtonBox QPushButton {
            background-color: #007aff;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 28px;
            font-size: 17px;
            font-weight: 600;
            min-width: 120px;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
        }
        
        QDialogButtonBox QPushButton:hover {
            background-color: #0056b3;
        }
        
        QDialogButtonBox QPushButton:pressed {
            background-color: #004494;
            transform: scale(0.98);
        }
        
        QDialogButtonBox QPushButton[text="Cancel"] {
            background-color: #8e8e93;
            color: white;
        }
        
        QDialogButtonBox QPushButton[text="Cancel"]:hover {
            background-color: #6d6d70;
        }
    )";
    
    setStyleSheet(iosDialogStyle);
    
    // 设置窗口属性
    setFixedSize(480, 450);  // 进一步增加高度从420到450
    setWindowTitle("连接到聊天室");
    setAttribute(Qt::WA_DeleteOnClose, false);
    setModal(true);
} 