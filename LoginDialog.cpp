#include "LoginDialog.h"
#include "ClickedLabel.h"
#include "ui_LoginDialog.h"
#include <qdebug.h>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    ui->user_label->setText("用户：");
    ui->pass_label->setText("密码：");
    ui->forget_label->setText("忘记密码？");
    ui->login_btn->setText("登录");
    ui->reg_btn->setText("注册");
    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);

    ui->forget_label->setState("normal", "hover", "", "selected", "selected_hover", "");
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slotForgetPwd);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDialog";
    delete ui;
}

void LoginDialog::slotForgetPwd()
{
    qDebug() << "slotForgetPwd";
    emit switchReset();
}