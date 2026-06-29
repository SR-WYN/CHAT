#include "SelfInfomation.h"
#include "ConfigMgr.h"
#include "HeartBeatMgr.h"
#include "HttpMgr.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "global.h"
#include "ui_SelfInfomation.h"

SelfInfomation::SelfInfomation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelfInfomation),
    _dragging(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setWindowTitle("SelfInfomation");
    setAttribute(Qt::WA_StyledBackground);

    // 加载当前帐号个人信息并禁用输入
    auto profile = UserMgr::getInstance().getSelfProfile();
    if (profile)
    {
        ui->name_edit->setText(profile->loginName);
        ui->nick_edit->setText(profile->nick);
        ui->sex_edit->setText(profile->sex == 1 ? QStringLiteral("男") : QStringLiteral("女"));
        ui->email_edit->setText(profile->email);
    }

    ui->name_edit->setReadOnly(true);
    ui->nick_edit->setReadOnly(true);
    ui->sex_edit->setReadOnly(true);
    ui->email_edit->setReadOnly(true);
}

SelfInfomation::~SelfInfomation()
{
    delete ui;
}

void SelfInfomation::on_quit_login_btn_clicked()
{
    int uid = UserMgr::getInstance().getUid();
    LOGI(LogModule::Ui, "quit login clicked uid={}", uid);

    // 1. 通知 GateServer 后端清理会话与 Redis 数据
    if (uid > 0)
    {
        QJsonObject json_obj;
        json_obj["uid"] = uid;
        HttpMgr::getInstance().postHttpReq(
            QUrl(ConfigMgr::getInstance().getUrlPrefix() + "/user_logout"), json_obj,
            ReqId::ID_LOGOUT_USER, Modules::LOGINMOD);
    }

    // 2. 停止心跳并断开 TCP 连接
    HeartBeatMgr::getInstance().stop();
    TcpMgr::getInstance().slot_heartbeat_abort();

    // 3. 清理本地状态并切换回登录界面
    UserMgr::getInstance().clearSession();
    emit sig_switch_login();
}

void SelfInfomation::on_back_chat_btn_clicked()
{
    emit sig_back_chat();
}

void SelfInfomation::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        _drag_pos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        _dragging = true;
        event->accept();
    }
}

void SelfInfomation::mouseMoveEvent(QMouseEvent *event)
{
    if (_dragging && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPosition().toPoint() - _drag_pos);
        event->accept();
    }
}

void SelfInfomation::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        _dragging = false;
        event->accept();
    }
}
