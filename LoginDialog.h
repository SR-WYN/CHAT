#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <qmap.h>
#include "global.h"

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
    void initHttpHandlers();
    void initHead();
    Ui::LoginDialog *ui;
    bool checkEmailValid();
    bool checkPwdValid();
    void AddTipErr(TipErr err, QString tips);
    void DelTipErr(TipErr err);
    QMap<TipErr,QString> _tip_errs;
    void showTip(QString str,bool b_ok);
    void enableBtn(bool enable);
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;
public slots:
    void slotForgetPwd();
private slots:
    void on_login_btn_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo si);
};

#endif // LOGINDIALOG_H
