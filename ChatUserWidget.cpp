#include "ChatUserWidget.h"
#include "global.h"
#include "ui_ChatUserWidget.h"
#include <qnamespace.h>
#include "UserData.h"

ChatUserWidget::ChatUserWidget(QWidget *parent) : ListItemBase(parent), ui(new Ui::ChatUserWidget)
{
    ui->setupUi(this);
    setItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWidget::~ChatUserWidget()
{
    delete ui;
}

QSize ChatUserWidget::sizeHint() const
{
    return QSize(250, 70);
}

void ChatUserWidget::setInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    QString icon = _user_info->_icon;
    if (icon.isEmpty())
    {
        icon = QStringLiteral(":/res/head_1.png");
    }
    QPixmap pixmap(icon);
    ui->icon_label->setPixmap(
        pixmap.scaled(ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);
    ui->user_name_label->setText(_user_info->displayName());
    ui->user_chat_label->setText(_user_info->_last_msg);
}

std::shared_ptr<UserInfo> ChatUserWidget::getUserInfo() const
{
    return _user_info;
}

void ChatUserWidget::updateLastMsg(const std::vector<std::shared_ptr<TextChatData>> &msg_vec)
{
    QString last_msg = "";
    for (auto &msg : msg_vec)
    {
        last_msg = msg->_msg_content;
        _user_info->_chat_msgs.push_back(msg);
    }   
    _user_info->_last_msg = last_msg;
    ui->user_chat_label->setText(last_msg);
}