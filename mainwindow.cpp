#include "mainwindow.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "ui_mainwindow.h"
#include <qnamespace.h>
#include "ResetDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/res/icon.ico"));
    _login_dlg = new LoginDialog(this);
    _reg_dlg = new RegisterDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _reg_dlg->hide();
    setCentralWidget(_login_dlg);
    _login_dlg->show();

    // 创建和注册消息连接
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchReg);

    //连接登录界面忘记密码信号
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::slotSwitchReset);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotSwitchReg()
{
    _reg_dlg = new RegisterDialog(this);

    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    connect(_reg_dlg,&RegisterDialog::sigSwitchLogin,this,&MainWindow::slotSwitchLogin);
    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
}

void MainWindow::slotSwitchLogin()
{
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _reg_dlg->hide();
    _login_dlg->show();
    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::slotSwitchReg);
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::slotSwitchReset);
}

void MainWindow::slotSwitchReset()
{
    // 创建一个CentralWidget,并将其设置为MainWindow的中心部件
    _reset_dlg = new ResetDialog(this);
    _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_reset_dlg);

    _login_dlg->hide();
    _reset_dlg->show();

    //注册返回登录界面信号槽函数
    connect(_reset_dlg,&ResetDialog::switchLogin,this,&MainWindow::slotSwitchLoginFromReset);
}

void MainWindow::slotSwitchLoginFromReset()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

   _reset_dlg->hide();
    _login_dlg->show();
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::slotSwitchReset);
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchReg);
}