#include "MainWindow.h"
#include "ChatDialog.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "ResetDialog.h"
#include "TcpMgr.h"
#include "ui_MainWindow.h"
#include <qnamespace.h>
#include <qwidget.h>

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
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_switch_chatdlg, this,
            &MainWindow::slot_chat_dlg_switch_chat);
            
//     emit TcpMgr::getInstancePtr()->sig_switch_chatdlg();//临时测试用
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

void MainWindow::slot_chat_dlg_switch_chat()
{
    _chat_dlg = new ChatDialog(this);
    _chat_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_chat_dlg);
    _chat_dlg->show();
    _login_dlg->hide();
    // 与 ChatDialog.ui 的最小尺寸一致，避免沿用登录窗的窄宽度导致聊天区被压扁；
    // 并主动 resize，使“首屏可见区域”等于设计布局，而不只提高 setMinimumSize。
    this->setMinimumSize(1200, 800);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->resize(1200, 800);
}