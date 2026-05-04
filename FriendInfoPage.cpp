#include "FriendInfoPage.h"
#include "ui_FriendInfoPage.h"
#include <memory>
#include <qpixmap.h>
#include <QStringLiteral>
#include "UserData.h"

FriendInfoPage::FriendInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendInfoPage)
{
    ui->setupUi(this);
    ui->msg_chat_button->setState("normal","hover","press");
    ui->video_chat_button->setState("normal","hover","press");
    ui->voice_chat_button->setState("normal","hover","press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;
}

void FriendInfoPage::setInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    QString icon = _user_info->_icon;
    if (icon.isEmpty())
    {
        icon = QStringLiteral(":/res/head_1.png");
    }
    QPixmap pixmap(icon);
    ui->icon_label->setPixmap(pixmap.scaled(ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);

    ui->name_tip->setText(user_info->_name);
    ui->alias_tip->setText(user_info->_alias);
}

void FriendInfoPage::on_msg_chat_button_clicked()
{
    qDebug() << "on_msg_chat_button_clicked";
    emit sig_jump_chat_item(_user_info);
}