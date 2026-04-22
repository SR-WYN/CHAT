#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <qmap.h>
#include "global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class LoginDialog;
}
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
    Ui::LoginDialog *_ui;
    bool checkEmailValid();
    bool checkPwdValid();
    void addTipErr(TipErr err, QString tips);
    void delTipErr(TipErr err);
    QMap<TipErr, QString> _tip_errs;
    void showTip(QString str, bool b_ok);
    void enableBtn(bool enable);
    QMap<ReqId, std::function<void(const QJsonObject &)>> _handlers;
    int _uid;
    QString _token;
public slots:
    void slot_forget_label_clicked();
private slots:
    void slot_login_btn_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_tcp_con_success(bool bsuccess);
    void slot_login_failed(int err);
signals:
    void sig_login_switch_register();
    void sig_login_switch_reset();
    void sig_login_connect_tcp(ServerInfo si);
};

#endif // LOGINDIALOG_H
