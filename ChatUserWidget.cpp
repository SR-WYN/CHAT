#include "ChatUserWidget.h"
#include "UserData.h"
#include "UserModels.h"
#include "global.h"
#include "ui_ChatUserWidget.h"
#include <qnamespace.h>

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

void ChatUserWidget::setInfo(std::shared_ptr<FriendListEntry> entry)
{
    _entry = std::move(entry);
    QString icon = _entry->profile.icon;
    if (icon.isEmpty())
    {
        icon = QStringLiteral(":/res/head_1.png");
    }
    QPixmap pixmap(icon);
    ui->icon_label->setPixmap(
        pixmap.scaled(ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);
    ui->user_name_label->setText(_entry->listDisplayName());
    ui->user_chat_label->setText(_entry->lastMessage);
}

std::shared_ptr<FriendListEntry> ChatUserWidget::getFriendEntry() const
{
    return _entry;
}

void ChatUserWidget::updateLastMsg(const std::vector<std::shared_ptr<TextChatData>> &msg_vec)
{
    if (msg_vec.empty())
    {
        return;
    }
    _entry->lastMessage = msg_vec.back()->_msg_content;
    ui->user_chat_label->setText(_entry->lastMessage);
}
