#include "RegisterDialog.h"
#include "ConfigMgr.h"
#include "HttpMgr.h"
#include "TimerBtn.h"
#include "global.h"
#include "ui_RegisterDialog.h"
#include <qdebug.h>
#include <qjsonobject.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qregularexpression.h>
#include <qtimer.h>

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegisterDialog),_countdown(5)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reg_mod_finish, this,
            &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
    ui->err_tip->clear();

    connect(ui->user_edit, &QLineEdit::editingFinished, this, [this]() {
        checkUserValid();
    });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this]() {
        checkEmailValid();
    });
    connect(ui->pass_edit, &QLineEdit::editingFinished, this, [this]() {
        checkPassValid();
    });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this]() {
        checkConfirmValid();
    });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this]() {
        checkVerifyValid();
    });

    ui->pass_visible->setCursor(Qt::PointingHandCursor);
    ui->confirm_visible->setCursor(Qt::PointingHandCursor);
    ui->pass_visible->setState("unvisible", "unvisible_hover", "", "visible", "visible_hover", "");
    ui->confirm_visible->setState("unvisible", "unvisible_hover", "", "visible", "visible_hover",
                                  "");

    connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->pass_visible->getCurState();
        if (state == ClickLabelState::NORMAL)
        {
            ui->pass_edit->setEchoMode(QLineEdit::Password);
        }
        else
        {
            ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->confirm_visible->getCurState();
        if (state == ClickLabelState::NORMAL)
        {
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        }
        else
        {
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    _countdown_timer = new QTimer(this);
    connect(_countdown_timer, &QTimer::timeout, [this]() {
        if (_countdown == 0)
        {
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
        auto str = QString("注册成功，%1 秒后自动跳转登录页").arg(_countdown);
        ui->tip_label->setText(str);
    });
}

RegisterDialog::~RegisterDialog()
{
    qDebug() << "destruct RegisterDialog";
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
        HttpMgr::GetInstance().PostHttpReq(
            QUrl(ConfigMgr::GetInstance().GetUrlPrefix() + "/get_verify_code"), json_obj,
            ReqId::ID_GET_VERIFY_CODE, Modules::REGISTERMOD);
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
    _handlers.insert(ReqId::ID_GET_VERIFY_CODE, [this](const QJsonObject &jsonObj) {
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
    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS)
        {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug() << "Email is " << email << Qt::endl;
        ChangeTipPage();
    });
}

void RegisterDialog::on_confirm_btn_clicked()
{
    if (ui->user_edit->text() == "")
    {
        showTip(tr("用户名不能为空"), false);
        return;
    }
    if (ui->email_edit->text() == "")
    {
        showTip(tr("邮箱不能为空"), false);
        return;
    }
    if (ui->pass_edit->text() == "")
    {
        showTip(tr("密码不能为空"), false);
        return;
    }
    if (ui->confirm_edit->text() == "")
    {
        showTip(tr("确认密码不能为空"), false);
        return;
    }
    if (ui->pass_edit->text() != ui->confirm_edit->text())
    {
        showTip(tr("两次密码不一致"), false);
        return;
    }
    if (ui->verify_edit->text() == "")
    {
        showTip(tr("验证码不能为空"), false);
        return;
    }

    // 发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    auto passwd = ui->pass_edit->text(); // 加密
    json_obj["passwd"] = xorString(passwd);
    auto confirm = ui->confirm_edit->text(); // 加密
    json_obj["confirm"] = xorString(confirm);
    json_obj["verify_code"] = ui->verify_edit->text();
    HttpMgr::GetInstance().PostHttpReq(
        QUrl(ConfigMgr::GetInstance().GetUrlPrefix() + "/user_register"), json_obj,
        ReqId::ID_REG_USER, Modules::REGISTERMOD);
}

bool RegisterDialog::checkUserValid()
{
    if (ui->user_edit->text() == "")
    {
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::checkEmailValid()
{
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch();
    if (!match)
    {
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->pass_edit->text();
    QRegularExpression regex(R"(^[a-zA-Z0-9~!@#$%^&*()-_=+\[\]{};':",./<>?\\|/.]{6,}$)");

    bool match = regex.match(pass).hasMatch();
    if (!match)
    {
        // 假设你在 TipErr 枚举中定义了 TIP_PWD_ERR
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度至少为6位且不能包含非法字符"));
        return false;
    }
    auto confirm = ui->confirm_edit->text();
    if (pass != confirm)
    {
        AddTipErr(TipErr::TIP_PWD_ERR, tr("两次密码不一致"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::checkConfirmValid()
{
    auto confirm = ui->confirm_edit->text();
    if (confirm != ui->pass_edit->text())
    {
        AddTipErr(TipErr::TIP_PWD_ERR, tr("两次密码不一致"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::checkVerifyValid()
{
    auto verify = ui->verify_edit->text();
    if (verify == "")
    {
        AddTipErr(TipErr::TIP_VERIFY_ERR, tr("验证码不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_VERIFY_ERR);
    return true;
}

void RegisterDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if (_tip_errs.empty())
    {
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void RegisterDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    _countdown_timer->start(1000);
}

void RegisterDialog::on_return_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

void RegisterDialog::on_cancel_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}