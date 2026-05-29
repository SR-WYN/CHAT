#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class RegisterDialog;
}
QT_END_NAMESPACE

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog() override;

private slots:
    void slot_get_code_btn_clicked();
    void slot_confirm_btn_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_return_btn_clicked();
    void slot_cancel_btn_clicked();

private:
    void initHttpHandlers();
    void showTip(const QString &str, bool b_ok);
    Ui::RegisterDialog *_ui;
    QMap<ReqId, std::function<void(const QJsonObject &)>> _handlers;
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkConfirmValid();
    bool checkVerifyValid();
    void addTipErr(TipErr te, QString tips);
    void delTipErr(TipErr te);
    void changeTipPage();
    QMap<TipErr, QString> _tip_errs;
    QTimer *_countdown_timer;
    int _countdown;
signals:
    void sig_register_switch_login();
};

#endif // REGISTERDIALOG_H
