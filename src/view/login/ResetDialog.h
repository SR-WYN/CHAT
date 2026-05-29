#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include "global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class ResetDialog;
}
QT_END_NAMESPACE

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog() override;
private slots:
    void slot_return_btn_clicked();
    void slot_get_verify_btn_clicked();
    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_confirm_btn_clicked();

private:
    bool checkUserValid();
    bool checkPassValid();
    void showTip(QString str, bool b_ok);
    bool checkEmailValid();
    bool checkVerifyValid();
    void addTipErr(TipErr te, QString tips);
    void delTipErr(TipErr te);
    void initHandlers();
    Ui::ResetDialog *_ui;
    QMap<TipErr, QString> _tip_errs;
    QMap<ReqId, std::function<void(const QJsonObject &)>> _handlers;
signals:
    void sig_reset_switch_login();
};

#endif // RESETDIALOG_H
