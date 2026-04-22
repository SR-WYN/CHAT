#include "MainWindow.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "ui_MainWindow.h"
#include <qnamespace.h>
#include "ResetDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    this->setWindowIcon(QIcon(":/res/icon.ico"));
    _login_dlg = new LoginDialog(this);
    _reg_dlg = new RegisterDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _reg_dlg->hide();
    setCentralWidget(_login_dlg);
    _login_dlg->show();

    connect(_login_dlg, &LoginDialog::sig_login_switch_register, this,
            &MainWindow::slot_login_dlg_switch_register);
    connect(_login_dlg, &LoginDialog::sig_login_switch_reset, this,
            &MainWindow::slot_login_dlg_switch_reset);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::slot_login_dlg_switch_register()
{
    _reg_dlg = new RegisterDialog(this);
    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    connect(_reg_dlg, &RegisterDialog::sig_register_switch_login, this,
            &MainWindow::slot_register_dlg_switch_login);
    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
}

void MainWindow::slot_register_dlg_switch_login()
{
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _reg_dlg->hide();
    _login_dlg->show();
    connect(_login_dlg, &LoginDialog::sig_login_switch_register, this,
            &MainWindow::slot_login_dlg_switch_register);
    connect(_login_dlg, &LoginDialog::sig_login_switch_reset, this,
            &MainWindow::slot_login_dlg_switch_reset);
}

void MainWindow::slot_login_dlg_switch_reset()
{
    _reset_dlg = new ResetDialog(this);
    _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_reset_dlg);

    _login_dlg->hide();
    _reset_dlg->show();
    connect(_reset_dlg, &ResetDialog::sig_reset_switch_login, this,
            &MainWindow::slot_reset_dlg_switch_login);
}

void MainWindow::slot_reset_dlg_switch_login()
{
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reset_dlg->hide();
    _login_dlg->show();
    connect(_login_dlg, &LoginDialog::sig_login_switch_reset, this,
            &MainWindow::slot_login_dlg_switch_reset);
    connect(_login_dlg, &LoginDialog::sig_login_switch_register, this,
            &MainWindow::slot_login_dlg_switch_register);
}
