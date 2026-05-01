#ifndef FINDFAILDIALOG_H
#define FINDFAILDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FindFailDialog;
}
QT_END_NAMESPACE

class FindFailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindFailDialog(QWidget *parent = nullptr);
    ~FindFailDialog() override;

private:
    Ui::FindFailDialog *ui;

private slots:
    void on_fail_sure_btn_clicked();
};

#endif // FINDFAILDIALOG_H
