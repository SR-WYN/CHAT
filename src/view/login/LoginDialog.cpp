#include "LoginDialog.h"
#include "AnimatedStateWidget.h"
#include "ConfigMgr.h"
#include "HttpMgr.h"
#include "Log.h"
#include "LogModule.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "global.h"
#include "ui_LoginDialog.h"
#include <qdebug.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), _ui(new Ui::LoginDialog)
{
    _ui->setupUi(this);

    connect(_ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::sig_login_switch_register);

    _ui->forget_label->setState("normal", "hover", "", "selected", "selected_hover", "");
    _ui->forget_label->setQssInteraction(AnimatedStateWidget::QssInteraction::ToggleSelection);
    connect(_ui->forget_label, &AnimatedStateWidget::clicked, this,
            &LoginDialog::slot_forget_label_clicked);
    connect(_ui->login_btn, &QPushButton::clicked, this, &LoginDialog::slot_login_btn_clicked);
    initHead();
    initHttpHandlers();
    connect(HttpMgr::getInstancePtr(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);
    connect(this, &LoginDialog::sig_login_connect_tcp, TcpMgr::getInstancePtr(),
            &TcpMgr::slot_tcp_connect);
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_con_success, this,
            &LoginDialog::slot_tcp_con_success);
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_login_failed, this,
            &LoginDialog::slot_login_failed);
}

LoginDialog::~LoginDialog()
{
    delete _ui;
}

void LoginDialog::slot_forget_label_clicked()
{
    emit sig_login_switch_reset();
}

void LoginDialog::initHttpHandlers()
{
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS)
        {
            QString tip = tr("登录失败，请稍后重试");
            switch (error)
            {
            case ErrorCodes::PASSWD_NOT_MATCH:
                tip = tr("邮箱或密码错误");
                break;
            case ErrorCodes::ERROR_JSON:
                tip = tr("请求参数错误");
                break;
            case ErrorCodes::RPC_FAILED:
                tip = tr("服务暂时不可用，请稍后重试");
                break;
            default:
                tip = tr("登录失败，错误码: %1").arg(error);
                break;
            }
            LOGE(LogModule::Ui, "login http failed error={} tip={}", error, tip.toStdString());
            showTip(tip, false);
            enableBtn(true);
            return;
        }
        auto email = jsonObj["email"].toString();
        ServerInfo si;
        si.uid = jsonObj["uid"].toInt();
        si.host = jsonObj["host"].toString();
        si.port = jsonObj["port"].toString();
        si.token = jsonObj["token"].toString();
        _uid = si.uid;
        _token = si.token;
        LOGI(LogModule::Ui, "login http success uid={} chat_server={}:{} token_len={}", si.uid,
             si.host.toStdString(), si.port.toStdString(), si.token.length());

        // 持久化凭据与服务端地址，供断线重连使用
        UserMgr::getInstance().setLastCredentials(si.uid, si.token);
        UserMgr::getInstance().setLastServerInfo(si);

        emit sig_login_connect_tcp(si);
    });
}

void LoginDialog::initHead()
{
    QPixmap originalPixmap(":/res/login.png");

    originalPixmap = originalPixmap.scaled(_ui->head_label->size(), Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10);
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, originalPixmap);

    _ui->head_label->setPixmap(roundedPixmap);
}

void LoginDialog::slot_login_btn_clicked()
{
    if (!checkEmailValid() || !checkPwdValid())
    {
        return;
    }
    enableBtn(false);

    auto email = _ui->email_edit->text();
    auto pwd = _ui->pass_edit->text();
    LOGI(LogModule::Ui, "login button clicked email={}", email.toStdString());
    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::getInstance().postHttpReq(
        QUrl(ConfigMgr::getInstance().getUrlPrefix() + "/user_login"), json_obj,
        ReqId::ID_LOGIN_USER, Modules::LOGINMOD);
}

bool LoginDialog::checkEmailValid()
{
    QString email = _ui->email_edit->text();
    if (email.isEmpty())
    {
        addTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱不能为空"));
        return false;
    }
    delTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::checkPwdValid()
{
    auto pass = _ui->pass_edit->text();
    if (pass.length() < 6 || pass.length() > 15)
    {
        addTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if (!match)
    {
        addTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;
    }
    delTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

void LoginDialog::addTipErr(TipErr err, QString tips)
{
    _tip_errs[err] = tips;
    showTip(tips, false);
}

void LoginDialog::delTipErr(TipErr err)
{
    _tip_errs.remove(err);
    if (_tip_errs.empty())
    {
        _ui->tip_err->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void LoginDialog::showTip(QString str, bool b_ok)
{
    if (b_ok)
    {
        _ui->tip_err->setProperty("state", "normal");
    }
    else
    {
        _ui->tip_err->setProperty("state", "err");
    }
    _ui->tip_err->setText(str);
    repolish(_ui->tip_err);
}

void LoginDialog::enableBtn(bool enable)
{
    _ui->login_btn->setEnabled(enable);
    _ui->reg_btn->setEnabled(enable);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS)
    {
        LOGE(LogModule::Ui, "login network error req_id={} err={}", static_cast<int>(id),
             static_cast<int>(err));
        showTip(tr("网络请求错误"), false);
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull())
    {
        LOGE(LogModule::Ui, "login response json parse failed: {}", res.toStdString());
        showTip(tr("json解析错误"), false);
        return;
    }
    if (!jsonDoc.isObject())
    {
        LOGE(LogModule::Ui, "login response is not object: {}", res.toStdString());
        showTip(tr("json解析错误"), false);
        return;
    }
    _handlers[id](jsonDoc.object());
}

void LoginDialog::slot_tcp_con_success(bool bsuccess)
{
    if (bsuccess)
    {
        showTip(tr("聊天服务连接成功，正在登陆..."), true);
        QJsonObject json_obj;
        json_obj["token"] = _token;

        QJsonDocument doc(json_obj);
        QString json_string = doc.toJson(QJsonDocument::Indented);

        LOGI(LogModule::Ui, "tcp connected, sending chat login token_len={}", _token.length());
        emit TcpMgr::getInstance().sig_send_data(ReqId::ID_CHAT_LOGIN, json_string);
    }
    else
    {
        LOGE(LogModule::Ui, "tcp connection failed");
        showTip(tr("网络异常"), false);
        enableBtn(true);
    }
}

void LoginDialog::slot_login_failed(int err)
{
    QString result = QString("登陆失败, err is %1").arg(err);
    LOGE(LogModule::Ui, "chat login failed err={}", err);
    showTip(result, false);
    enableBtn(true);
}
