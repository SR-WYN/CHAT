#include "LoginDialog.h"
#include "ClickedLabel.h"
#include "ConfigMgr.h"
#include "HttpMgr.h"
#include "global.h"
#include "ui_LoginDialog.h"
#include <qdebug.h>
#include <qjsonobject.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);

    ui->forget_label->setState("normal", "hover", "", "selected", "selected_hover", "");
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slotForgetPwd);
    initHead();
    initHttpHandlers();
    // 连接登录回包信号
    connect(HttpMgr::getInstancePtr(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);
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
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();

        qDebug() << "email is " << email << "\n uid is " << si.Uid << "\n host is " << si.Host
                 << "\n Port is " << si.Port << "\n Token is " << si.Token;
        emit sig_connect_tcp(si);
    });
}

void LoginDialog::initHead()
{
    // 加载图片
    QPixmap originalPixmap(":/res/login.png");

    // 设置图片自动缩放
    qDebug() << originalPixmap.size() << ui->head_label->size();
    originalPixmap = originalPixmap.scaled(ui->head_label->size(), Qt::KeepAspectRatio,
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
    ui->head_label->setPixmap(roundedPixmap);
}

void LoginDialog::on_login_btn_clicked()
{
    qDebug() << "on_login_btn_clicked";
    if (!checkEmailValid() || !checkPwdValid())
    {
        return;
    }
    enableBtn(false);

    auto email = ui->email_edit->text();
    auto pwd = ui->pass_edit->text();
    // 发送http请求登录
    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::GetInstance().PostHttpReq(
        QUrl(ConfigMgr::GetInstance().GetUrlPrefix() + "/user_login"), json_obj,
        ReqId::ID_LOGIN_USER, Modules::LOGINMOD);
}

bool LoginDialog::checkEmailValid()
{
    QString email = ui->email_edit->text();
    if (email.isEmpty())
    {
        qDebug() << "email empty ";
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::checkPwdValid()
{
    auto pass = ui->pass_edit->text();
    if (pass.length() < 6 || pass.length() > 15)
    {
        // 提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
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
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

void LoginDialog::AddTipErr(TipErr err, QString tips)
{
    _tip_errs[err] = tips;
    showTip(tips, false);
}

void LoginDialog::DelTipErr(TipErr err)
{
    _tip_errs.remove(err);
    if (_tip_errs.empty())
    {
        ui->tip_err->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void LoginDialog::showTip(QString str, bool b_ok)
{
    if (b_ok)
    {
        ui->tip_err->setProperty("state", "normal");
    }
    else
    {
        ui->tip_err->setProperty("state", "err");
    }
    ui->tip_err->setText(str);
    repolish(ui->tip_err);
}

void LoginDialog::enableBtn(bool enable)
{
    ui->login_btn->setEnabled(enable);
    ui->reg_btn->setEnabled(enable);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS)
    {
        showTip(tr("网络请求错误"),false);
        return;
    }
    // 解析 JSON 字符串，res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    // json 解析错误
    if (jsonDoc.isNull())
    {
        showTip(tr("json解析错误"),false);
        return;
    }
    // json 解析错误
    if (!jsonDoc.isObject())
    {
        showTip(tr("json解析错误"),false);
        return;
    }
    //调用对应逻辑，根据id回调
    _handlers[id](jsonDoc.object());
}