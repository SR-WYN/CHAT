#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterDialog; }
QT_END_NAMESPACE

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog() override;


private slots:
    void on_get_code_clicked();

private:
    void showTip(const QString &str, bool b_ok);
    Ui::RegisterDialog *ui;
};

#endif // REGISTERDIALOG_H
