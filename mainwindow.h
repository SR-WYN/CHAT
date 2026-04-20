#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LoginDialog;
class RegisterDialog;
class ResetDialog;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
public slots:
    void slotSwitchReg();
    void slotSwitchLogin();
    void slotSwitchReset();
    void slotSwitchLoginFromReset();
private:
    Ui::MainWindow *ui;
    LoginDialog *_login_dlg;
    RegisterDialog *_reg_dlg;
    ResetDialog *_reset_dlg;
};
#endif // MAINWINDOW_H
