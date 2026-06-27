#include "MainWindow.h"
#include "ChatDialog.h"
#include "HeartBeatMgr.h"
#include "LoginDialog.h"
#include "Log.h"
#include "LogModule.h"
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
    LOGI(LogModule::Ui, "switch to self info");
    this->hide();
    if (_self_info == nullptr)
    {
        _self_info = new SelfInfomation(nullptr);
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
    LOGI(LogModule::Ui, "self info switch to login");
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
    LOGI(LogModule::Ui, "switch to register dialog");
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
    LOGI(LogModule::Ui, "register dialog switch to login");
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
    LOGI(LogModule::Ui, "switch to reset dialog");
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
    LOGI(LogModule::Ui, "reset dialog switch to login");
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
    LOGI(LogModule::Ui, "switch to chat dialog");
    _chat_dlg = new ChatDialog(this);
    _chat_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    connect(_chat_dlg, &ChatDialog::sig_switch_login, this, &MainWindow::slot_self_info_switch_login);
    connect(_chat_dlg, &ChatDialog::sig_switch_self_info, this, &MainWindow::slot_switch_self_info);
    setCentralWidget(_chat_dlg);
    _chat_dlg->show();
    _login_dlg->hide();
    this->setMinimumSize(1200, 800);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->resize(1200, 800);
}

void MainWindow::slot_heartbeat_timeout_back_to_login()
{
    LOGW(LogModule::Ui, "heartbeat timeout, back to login");
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
