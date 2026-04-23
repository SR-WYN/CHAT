#include "LoginDialog.h"
#include "ClickedLabel.h"
#include "ConfigMgr.h"
#include "HttpMgr.h"
#include "TcpMgr.h"
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
    connect(_ui->forget_label, &ClickedLabel::sig_label_clicked, this,
            &LoginDialog::slot_forget_label_clicked);
    connect(_ui->login_btn, &QPushButton::clicked, this, &LoginDialog::slot_login_btn_clicked);
    initHead();
    initHttpHandlers();
    // 连接登录回包信号
    connect(HttpMgr::getInstancePtr(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);
    // 连接tcp连接信号
    connect(this, &LoginDialog::sig_login_connect_tcp, TcpMgr::getInstancePtr(),
            &TcpMgr::slot_tcp_connect);
    // 连接tcp管理者发出的连接成功信号
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_con_success, this,
            &LoginDialog::slot_tcp_con_success);
    // 连接tcp管理者发出的失败信号
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_login_failed, this,
            &LoginDialog::slot_login_failed);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDialog";
    delete _ui;
}

void LoginDialog::slot_forget_label_clicked()
{
    qDebug() << "slot_forget_label_clicked";
    emit sig_login_switch_reset();
}

void LoginDialog::initHttpHandlers()
{
    // 注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS)
        {
            showTip(tr("参数错误"), false);
            enableBtn(true);
            return;
        }
        // 发送信号通知tcpMgr发送长连接
        auto email = jsonObj["email"].toString();
        ServerInfo si;
        si.uid = jsonObj["uid"].toInt();
        si.host = jsonObj["host"].toString();
        si.port = jsonObj["port"].toString();
        si.token = jsonObj["token"].toString();
        _uid = si.uid;
        _token = si.token;

        qDebug() << "email is " << email << "\n uid is " << si.uid << "\n host is " << si.host
                 << "\n Port is " << si.port << "\n Token is " << si.token;
        emit sig_login_connect_tcp(si);
    });
}

void LoginDialog::initHead()
{
    // 加载图片
    QPixmap originalPixmap(":/res/login.png");

    // 设置图片自动缩放
    qDebug() << originalPixmap.size() << _ui->head_label->size();
    originalPixmap = originalPixmap.scaled(_ui->head_label->size(), Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

    // 创建一个和原始图片相同大小的QPixmap，同于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent); // 填充透明背景

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 使用QPainterPath绘制圆角图片
    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10);
    painter.setClipPath(path);

    // 将原始图片绘制到roundedPixmap上
    painter.drawPixmap(0, 0, originalPixmap);

    // 设置绘制好的圆角图片到QLabel
    _ui->head_label->setPixmap(roundedPixmap);
}

void LoginDialog::slot_login_btn_clicked()
{
    qDebug() << "slot_login_btn_clicked";
    if (!checkEmailValid() || !checkPwdValid())
    {
        return;
    }
    enableBtn(false);

    auto email = _ui->email_edit->text();
    auto pwd = _ui->pass_edit->text();
    // 发送http请求登录
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
        qDebug() << "email empty ";
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
        showTip(tr("网络请求错误"), false);
        return;
    }
    // 解析 JSON 字符串，res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    // json 解析错误
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
    // 调用对应逻辑，根据id回调
    _handlers[id](jsonDoc.object());
}

void LoginDialog::slot_tcp_con_success(bool bsuccess)
{
    if (bsuccess)
    {
        showTip(tr("聊天服务连接成功，正在登陆..."), true);
        QJsonObject json_obj;
        json_obj["uid"] = _uid;
        json_obj["token"] = _token;

        QJsonDocument doc(json_obj);
        QString json_string = doc.toJson(QJsonDocument::Indented);

        // 发送tcp请求给chat server
        TcpMgr::getInstance().sig_send_data(ReqId::ID_CHAT_LOGIN, json_string);
    }
    else
    {
        showTip(tr("网络异常"), false);
        enableBtn(true);
    }
}

void LoginDialog::slot_login_failed(int err)
{
    QString relust = QString("登陆失败, err is %1").arg(err);
    showTip(relust,false);
    enableBtn(true);
}