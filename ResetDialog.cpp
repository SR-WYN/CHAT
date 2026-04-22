#include "ResetDialog.h"
#include "ConfigMgr.h"
#include "HttpMgr.h"
#include "ui_ResetDialog.h"
#include <QDebug>
#include <QRegularExpression>
#include <qdebug.h>

ResetDialog::ResetDialog(QWidget *parent) : QDialog(parent), _ui(new Ui::ResetDialog)
{
    _ui->setupUi(this);

    connect(_ui->return_btn, &QPushButton::clicked, this, &ResetDialog::slot_return_btn_clicked);
    connect(_ui->get_verify_btn, &QPushButton::clicked, this,
            &ResetDialog::slot_get_verify_btn_clicked);
    connect(_ui->confirm_btn, &QPushButton::clicked, this, &ResetDialog::slot_confirm_btn_clicked);

    connect(_ui->user_edit, &QLineEdit::editingFinished, this, [this]() {
        checkUserValid();
    });

    connect(_ui->email_edit, &QLineEdit::editingFinished, this, [this]() {
        checkEmailValid();
    });
    connect(_ui->pwd_edit, &QLineEdit::editingFinished, this, [this]() {
        checkPassValid();
    });
    connect(_ui->get_verify_edit, &QLineEdit::editingFinished, this, [this]() {
        checkVerifyValid();
    });

    initHandlers();

    connect(HttpMgr::getInstancePtr(), &HttpMgr::sig_reset_mod_finish, this,
            &ResetDialog::slot_reset_mod_finish);
}

ResetDialog::~ResetDialog()
{
    delete _ui;
}

bool ResetDialog::checkUserValid()
{
    if (_ui->user_edit->text() == "")
    {
        addTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }
    delTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool ResetDialog::checkPassValid()
{
    auto pass = _ui->pwd_edit->text();
    if (pass.length() < 6 || pass.length() > 15)
    {
        // 提示长度不准确
        addTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }
    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if (!match)
    {
        // 提示字符非法
        addTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;
    }
    delTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool ResetDialog::checkEmailValid()
{
    // 验证邮箱的地址正则表达式
    auto email = _ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if (!match)
    {
        // 提示邮箱不正确
        addTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }
    delTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool ResetDialog::checkVerifyValid()
{
    auto pass = _ui->get_verify_edit->text();
    if (pass.isEmpty())
    {
        addTipErr(TipErr::TIP_VERIFY_ERR, tr("验证码不能为空"));
        return false;
    }
    delTipErr(TipErr::TIP_VERIFY_ERR);
    return true;
}

void ResetDialog::addTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void ResetDialog::delTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if (_tip_errs.empty())
    {
        _ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void ResetDialog::showTip(QString str, bool b_ok)
{
    if (b_ok)
    {
        _ui->err_tip->setProperty("state", "normal");
    }
    else
    {
        _ui->err_tip->setProperty("state", "err");
    }
    _ui->err_tip->setText(str);
    repolish(_ui->err_tip);
}

void ResetDialog::slot_return_btn_clicked()
{
    qDebug() << "slot_return_btn_clicked";
    emit sig_reset_switch_login();
}

void ResetDialog::slot_get_verify_btn_clicked()
{
    qDebug() << "receive get verify btn clicked ";
    auto email = _ui->email_edit->text();
    auto bcheck = checkEmailValid();
    if (!bcheck)
    {
        return;
    }
    // 发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpMgr::getInstance().postHttpReq(
        QUrl(ConfigMgr::getInstance().getUrlPrefix() + "/get_verify_code"), json_obj,
        ReqId::ID_GET_VERIFY_CODE, Modules::RESETMOD);
}

void ResetDialog::initHandlers()
{
    // 注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VERIFY_CODE, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS)
        {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已发送到邮箱，注意查收"), true);
        qDebug() << "email is " << email;
    });
    // 注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_RESET_PWD, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS)
        {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("重置成功,点击返回登录"), true);
        qDebug() << "email is " << email;
        qDebug() << "user uuid is " << jsonObj["uuid"].toString();
    });
}

void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS)
    {
        showTip(tr("网络请求错误"), false);
        return;
    }
    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    // json解析错误
    if (jsonDoc.isNull())
    {
        showTip(tr("json解析错误"), false);
        return;
    }
    // json 解析错误
    if (!jsonDoc.isObject())
    {
        showTip(tr("json解析错误"), false);
        return;
    }
    // 调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());
    return;
}

void ResetDialog::slot_confirm_btn_clicked()
{
    bool valid = checkUserValid();
    if (!valid)
    {
        return;
    }
    valid = checkEmailValid();
    if (!valid)
    {
        return;
    }
    valid = checkPassValid();
    if (!valid)
    {
        return;
    }
    valid = checkVerifyValid();
    if (!valid)
    {
        return;
    }
    // 发送http重置用户请求
    QJsonObject json_obj;
    json_obj["user"] = _ui->user_edit->text();
    json_obj["email"] = _ui->email_edit->text();
    json_obj["passwd"] = xorString(_ui->pwd_edit->text());
    json_obj["verify_code"] = _ui->get_verify_edit->text();
    HttpMgr::getInstance().postHttpReq(
        QUrl(ConfigMgr::getInstance().getUrlPrefix() + "/reset_pwd"), json_obj, ReqId::ID_RESET_PWD,
        Modules::RESETMOD);
}
