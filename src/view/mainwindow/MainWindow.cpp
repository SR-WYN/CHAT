#include "MainWindow.h"
#include "ChatDialog.h"
#include "HeartBeatMgr.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "ResetDialog.h"
#include "SelfInfomation.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <qnamespace.h>
#include <qwidget.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow), _chat_dlg(nullptr), _self_info(nullptr)
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
    connect(&HeartBeatMgr::getInstance(), &HeartBeatMgr::sig_heartbeat_timeout, this,
            &MainWindow::slot_heartbeat_timeout_back_to_login, Qt::QueuedConnection);
}

void MainWindow::slot_switch_self_info()
{
    // 隐藏 MainWindow（包含 ChatDialog），弹出独立的个人信息窗口
    this->hide();
    if (_self_info == nullptr)
    {
        _self_info = new SelfInfomation(nullptr);
        // 使用 QueuedConnection 避免信号处理过程中对象被销毁
        connect(_self_info, &SelfInfomation::sig_switch_login, this, &MainWindow::slot_self_info_switch_login, Qt::QueuedConnection);
        connect(_self_info, &SelfInfomation::sig_back_chat, this, [this]() {
            _self_info->hide();
            _self_info->deleteLater();
            _self_info = nullptr;
            this->show();
        });
    }
    _self_info->resize(1000, 700);
    _self_info->show();
}

void MainWindow::slot_self_info_switch_login()
{
    UserMgr::getInstance().clearSession();
    if (_self_info != nullptr)
    {
        _self_info->hide();
        _self_info->deleteLater();
        _self_info = nullptr;
    }
    if (_chat_dlg != nullptr)
    {
        _chat_dlg->hide();
        _chat_dlg->deleteLater();
        _chat_dlg = nullptr;
    }
    // 先显示 MainWindow，再切换到登录页
    this->show();
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _login_dlg->show();
    connect(_login_dlg, &LoginDialog::sig_login_switch_register, this,
            &MainWindow::slot_login_dlg_switch_register);
    connect(_login_dlg, &LoginDialog::sig_login_switch_reset, this,
            &MainWindow::slot_login_dlg_switch_reset);
    this->setMinimumSize(300, 500);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->resize(500, 800);
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
    connect(_chat_dlg, &ChatDialog::sig_switch_login, this, &MainWindow::slot_self_info_switch_login);
    connect(_chat_dlg, &ChatDialog::sig_switch_self_info, this, &MainWindow::slot_switch_self_info);
    setCentralWidget(_chat_dlg);
    _chat_dlg->show();
    _login_dlg->hide();
    // 与 ChatDialog.ui 的最小尺寸一致，避免沿用登录窗的窄宽度导致聊天区被压扁；
    // 并主动 resize，使“首屏可见区域”等于设计布局，而不只提高 setMinimumSize。
    this->setMinimumSize(1200, 800);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->resize(1200, 800);
}

void MainWindow::slot_heartbeat_timeout_back_to_login()
{
    QMessageBox::warning(this, tr("连接已断开"), tr("与服务器的心跳超时，请重新登录。"));
    UserMgr::getInstance().clearSession();
    if (_chat_dlg != nullptr)
    {
        _chat_dlg->hide();
        _chat_dlg->deleteLater();
        _chat_dlg = nullptr;
    }
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _login_dlg->show();
    connect(_login_dlg, &LoginDialog::sig_login_switch_register, this,
            &MainWindow::slot_login_dlg_switch_register);
    connect(_login_dlg, &LoginDialog::sig_login_switch_reset, this,
            &MainWindow::slot_login_dlg_switch_reset);
    this->setMinimumSize(300, 500);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->resize(500, 800);
}
