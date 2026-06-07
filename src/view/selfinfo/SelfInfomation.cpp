#include "SelfInfomation.h"
#include "HeartBeatMgr.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "ui_SelfInfomation.h"

SelfInfomation::SelfInfomation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelfInfomation),
    _dragging(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setWindowTitle("SelfInfomation");
}

SelfInfomation::~SelfInfomation()
{
    delete ui;
}

void SelfInfomation::on_quit_login_btn_clicked()
{
    HeartBeatMgr::getInstance().stop();
    TcpMgr::getInstance().slot_heartbeat_abort();
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
