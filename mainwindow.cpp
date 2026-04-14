#include "mainwindow.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "ui_mainwindow.h"
#include <qnamespace.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  this->setWindowIcon(QIcon(":/res/icon.ico"));
  _login_dlg = new LoginDialog(this);
  _reg_dlg = new RegisterDialog(this);
  _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  _reg_dlg->hide();
  setCentralWidget(_login_dlg);
  _login_dlg->show();

  // 创建和注册消息连接
  connect(_login_dlg, &LoginDialog::switchRegister, this,
          &MainWindow::slotSwitchReg);
  
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::slotSwitchReg() {
  setCentralWidget(_reg_dlg);
  _login_dlg->hide();
  _reg_dlg->show();
}
