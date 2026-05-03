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
    ui->user_name_label->setText(_user_info->_name);
    ui->user_chat_label->setText(_user_info->_last_msg);
}