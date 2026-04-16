#include "RegisterDialog.h"
#include "HttpMgr.h"
#include "ui_RegisterDialog.h"
#include <qregularexpression.h>
#include "ConfigMgr.h"

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reg_mod_finish, this,
            &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch();
    if (match)
    {
        // 发送http验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance().PostHttpReq(QUrl(ConfigMgr::GetInstance().GetUrlPrefix() + "/get_verify_code"), json_obj,
                                           ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
    }
    else
    {
        showTip(tr("邮箱地址不正确"), false);
    }
}

void RegisterDialog::showTip(const QString &str, bool b_ok)
{
    if (b_ok)
    {
        ui->err_tip->setProperty("state", "normal");
    }
    else
    {
        ui->err_tip->setProperty("state", "error");
    }
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS)
    {
        showTip(tr("网络请求错误"), false);
        return;
    }
    // 解析json字符串，res转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull())
    {
        showTip(tr("json解析失败"), false);
        return;
    }
    // json 解析错误;
    if (!jsonDoc.isObject())
    {
        showTip(tr("json解析失败"), false);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;
}

void RegisterDialog::initHttpHandlers()
{
    // 注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE,
                     [this](const QJsonObject &jsonObj)
                     {
                         int error = jsonObj["error"].toInt();
                         if (error != ErrorCodes::SUCCESS)
                         {
                             showTip(tr("参数错误"), false);
                             return;
                         }
                         auto email = jsonObj["email"].toString();
                         showTip(tr("验证码已经发送到邮箱,注意查收"), true);
                         qDebug() << "Emailis" << email;
                     });
}