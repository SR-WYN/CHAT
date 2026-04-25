#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LoginDialog;
class RegisterDialog;
class ResetDialog;
class ChatDialog;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
public slots:
    void slot_login_dlg_switch_register();
    void slot_register_dlg_switch_login();
    void slot_login_dlg_switch_reset();
    void slot_reset_dlg_switch_login();
    void slot_chat_dlg_switch_chat();

private:
    Ui::MainWindow *_ui;
    LoginDialog *_login_dlg;
    RegisterDialog *_reg_dlg;
    ResetDialog *_reset_dlg;
    ChatDialog *_chat_dlg;
};
#endif // MAINWINDOW_H
