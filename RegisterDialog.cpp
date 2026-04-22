#include "RegisterDialog.h"
#include "ClickedLabel.h"
#include "ConfigMgr.h"
#include "HttpMgr.h"
#include "TimerBtn.h"
#include "global.h"
#include <QJsonDocument>
#include "ui_RegisterDialog.h"
#include <qdebug.h>
#include <qjsonobject.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qregularexpression.h>
#include <qtimer.h>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::RegisterDialog), _countdown(5)
{
    _ui->setupUi(this);
    _ui->pass_edit->setEchoMode(QLineEdit::Password);
    _ui->confirm_edit->setEchoMode(QLineEdit::Password);
    _ui->err_tip->setProperty("state", "normal");
    repolish(_ui->err_tip);
    connect(&HttpMgr::getInstance(), &HttpMgr::sig_reg_mod_finish, this,
            &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
    _ui->err_tip->clear();

    connect(_ui->get_code, &QPushButton::clicked, this, &RegisterDialog::slot_get_code_btn_clicked);
    connect(_ui->confirm_btn, &QPushButton::clicked, this,
            &RegisterDialog::slot_confirm_btn_clicked);
    connect(_ui->return_btn, &QPushButton::clicked, this, &RegisterDialog::slot_return_btn_clicked);
    connect(_ui->cancel_btn, &QPushButton::clicked, this, &RegisterDialog::slot_cancel_btn_clicked);

    connect(_ui->user_edit, &QLineEdit::editingFinished, this, [this]() {
        checkUserValid();
    });
    connect(_ui->email_edit, &QLineEdit::editingFinished, this, [this]() {
        checkEmailValid();
    });
    connect(_ui->pass_edit, &QLineEdit::editingFinished, this, [this]() {
        checkPassValid();
    });
    connect(_ui->confirm_edit, &QLineEdit::editingFinished, this, [this]() {
        checkConfirmValid();
    });
    connect(_ui->verify_edit, &QLineEdit::editingFinished, this, [this]() {
        checkVerifyValid();
    });

    _ui->pass_visible->setCursor(Qt::PointingHandCursor);
    _ui->confirm_visible->setCursor(Qt::PointingHandCursor);
    _ui->pass_visible->setState("unvisible", "unvisible_hover", "", "visible", "visible_hover", "");
    _ui->confirm_visible->setState("unvisible", "unvisible_hover", "", "visible", "visible_hover",
                                    "");

    connect(_ui->pass_visible, &ClickedLabel::sig_label_clicked, this, [this]() {
        auto state = _ui->pass_visible->getCurState();
        if (state == ClickLabelState::NORMAL)
        {
            _ui->pass_edit->setEchoMode(QLineEdit::Password);
        }
        else
        {
            _ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    connect(_ui->confirm_visible, &ClickedLabel::sig_label_clicked, this, [this]() {
        auto state = _ui->confirm_visible->getCurState();
        if (state == ClickLabelState::NORMAL)
        {
            _ui->confirm_edit->setEchoMode(QLineEdit::Password);
        }
        else
        {
            _ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    _countdown_timer = new QTimer(this);
    connect(_countdown_timer, &QTimer::timeout, [this]() {
        if (_countdown == 0)
        {
            _countdown_timer->stop();
            emit sig_register_switch_login();
            return;
        }
        _countdown--;
        auto str = QString("注册成功，%1 秒后自动跳转登录页").arg(_countdown);
        _ui->tip_label->setText(str);
    });
}

RegisterDialog::~RegisterDialog()
{
    qDebug() << "destruct RegisterDialog";
    delete _ui;
}

void RegisterDialog::slot_get_code_btn_clicked()
{
    auto email = _ui->email_edit->text();
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch();
    if (match)
    {
        // 发送http验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::getInstance().postHttpReq(
            QUrl(ConfigMgr::getInstance().getUrlPrefix() + "/get_verify_code"), json_obj,
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
        _ui->err_tip->setProperty("state", "normal");
    }
    else
    {
        _ui->err_tip->setProperty("state", "error");
    }
    _ui->err_tip->setText(str);
    repolish(_ui->err_tip);
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
        changeTipPage();
    });
}

void RegisterDialog::slot_confirm_btn_clicked()
{
    if (_ui->user_edit->text() == "")
    {
        showTip(tr("用户名不能为空"), false);
        return;
    }
    if (_ui->email_edit->text() == "")
    {
        showTip(tr("邮箱不能为空"), false);
        return;
    }
    if (_ui->pass_edit->text() == "")
    {
        showTip(tr("密码不能为空"), false);
        return;
    }
    if (_ui->confirm_edit->text() == "")
    {
        showTip(tr("确认密码不能为空"), false);
        return;
    }
    if (_ui->pass_edit->text() != _ui->confirm_edit->text())
    {
        showTip(tr("两次密码不一致"), false);
        return;
    }
    if (_ui->verify_edit->text() == "")
    {
        showTip(tr("验证码不能为空"), false);
        return;
    }

    // 发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = _ui->user_edit->text();
    json_obj["email"] = _ui->email_edit->text();
    auto passwd = _ui->pass_edit->text(); // 加密
    json_obj["passwd"] = xorString(passwd);
    auto confirm = _ui->confirm_edit->text(); // 加密
    json_obj["confirm"] = xorString(confirm);
    json_obj["verify_code"] = _ui->verify_edit->text();
    HttpMgr::getInstance().postHttpReq(
        QUrl(ConfigMgr::getInstance().getUrlPrefix() + "/user_register"), json_obj,
        ReqId::ID_REG_USER, Modules::REGISTERMOD);
}

bool RegisterDialog::checkUserValid()
{
    if (_ui->user_edit->text() == "")
    {
        addTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }
    delTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::checkEmailValid()
{
    auto email = _ui->email_edit->text();
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch();
    if (!match)
    {
        addTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }
    delTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::checkPassValid()
{
    auto pass = _ui->pass_edit->text();
    QRegularExpression regex(R"(^[a-zA-Z0-9~!@#$%^&*()-_=+\[\]{};':",./<>?\\|/.]{6,}$)");

    bool match = regex.match(pass).hasMatch();
    if (!match)
    {
        // 假设你在 TipErr 枚举中定义了 TIP_PWD_ERR
        addTipErr(TipErr::TIP_PWD_ERR, tr("密码长度至少为6位且不能包含非法字符"));
        return false;
    }
    auto confirm = _ui->confirm_edit->text();
    if (pass != confirm)
    {
        addTipErr(TipErr::TIP_PWD_ERR, tr("两次密码不一致"));
        return false;
    }
    delTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::checkConfirmValid()
{
    auto confirm = _ui->confirm_edit->text();
    if (confirm != _ui->pass_edit->text())
    {
        addTipErr(TipErr::TIP_PWD_ERR, tr("两次密码不一致"));
        return false;
    }
    delTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::checkVerifyValid()
{
    auto verify = _ui->verify_edit->text();
    if (verify == "")
    {
        addTipErr(TipErr::TIP_VERIFY_ERR, tr("验证码不能为空"));
        return false;
    }
    delTipErr(TipErr::TIP_VERIFY_ERR);
    return true;
}

void RegisterDialog::addTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::delTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if (_tip_errs.empty())
    {
        _ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void RegisterDialog::changeTipPage()
{
    _countdown_timer->stop();
    _ui->stackedWidget->setCurrentWidget(_ui->page_2);

    _countdown_timer->start(1000);
}

void RegisterDialog::slot_return_btn_clicked()
{
    _countdown_timer->stop();
    emit sig_register_switch_login();
}

void RegisterDialog::slot_cancel_btn_clicked()
{
    _countdown_timer->stop();
    emit sig_register_switch_login();
}
