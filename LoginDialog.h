#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog() override;


private:
    Ui::LoginDialog *ui;
public slots:
    void slotForgetPwd();
signals:
    void switchRegister();
    void switchReset();
};

#endif // LOGINDIALOG_H
