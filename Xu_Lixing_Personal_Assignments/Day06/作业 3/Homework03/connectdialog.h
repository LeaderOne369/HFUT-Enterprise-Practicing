#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ConnectDialog;
}
QT_END_NAMESPACE

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog();

    QString getServerHost() const;
    quint16 getServerPort() const;
    QString getNickname() const;
    bool isServerMode() const;

private slots:
    void onModeChanged();

private:
    void applyIOSStyle();
    
    Ui::ConnectDialog *ui;
};

#endif // CONNECTDIALOG_H 