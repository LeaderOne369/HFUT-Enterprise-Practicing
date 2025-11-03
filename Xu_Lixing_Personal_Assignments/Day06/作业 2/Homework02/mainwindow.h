#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkInterface>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QFrame>
#include <QFont>
#include <QProcess>
#include <QTimer>
#include <QStyleFactory>
#include <QApplication>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct NetworkCardInfo {
    QString name;
    QString displayName;
    QString macAddress;
    QStringList ipAddresses;
    QStringList subnetMasks;
    QString status;
    QString type;
    int mtu;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshNetworkCards();
    void onNetworkCardSelectionChanged();
    void addIPAddress();
    void removeIPAddress();
    void modifyIPAddress();
    void applyNetworkChanges();

private:
    Ui::MainWindow *ui;
    
    // UI Components
    QWidget *centralWidget;
    QSplitter *mainSplitter;
    
    // Left panel - Network card list
    QGroupBox *networkListGroup;
    QTableWidget *networkTable;
    QPushButton *refreshButton;
    
    // Right panel - Network configuration
    QGroupBox *configGroup;
    QLabel *selectedCardLabel;
    QLineEdit *cardNameEdit;
    QLabel *macAddressLabel;
    QLabel *statusLabel;
    QLabel *typeLabel;
    QLabel *mtuLabel;
    
    // IP configuration section
    QGroupBox *ipConfigGroup;
    QTableWidget *ipTable;
    QPushButton *addIpButton;
    QPushButton *removeIpButton;
    QPushButton *modifyIpButton;
    QPushButton *applyButton;
    
    // Data
    QList<NetworkCardInfo> networkCards;
    int currentSelectedCard;
    
    // Methods
    void setupUI();
    void setupStyles();
    void loadNetworkCards();
    void populateNetworkTable();
    void populateIPTable();
    void clearConfigPanel();
    bool executeNetworkCommand(const QString &command);
    QString getNetmaskFromPrefix(int prefix);
    int getPrefixFromNetmask(const QString &netmask);
};

#endif // MAINWINDOW_H
