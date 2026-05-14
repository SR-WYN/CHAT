#include "ConUserItem.h"
#include "UserModels.h"
#include "ui_ConUserItem.h"

ConUserItem::ConUserItem(QWidget *parent) : ListItemBase(parent), ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::CONTACT_USER_ITEM);
    ui->red_point->raise();
    showRedPoint(false);
}

ConUserItem::~ConUserItem()
{
    delete ui;
}

QSize ConUserItem::sizeHint() const
{
    return QSize(250, 70);
}

void ConUserItem::refreshUi()
{
    if (!_entry)
    {
        return;
    }
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
}

void ConUserItem::setInfo(std::shared_ptr<AuthAcceptedPeer> peer)
{
    _entry = std::make_shared<FriendListEntry>(*peer);
    refreshUi();
}

void ConUserItem::setInfo(UserProfile profile)
{
    _entry = std::make_shared<FriendListEntry>(std::move(profile));
    refreshUi();
}

void ConUserItem::showRedPoint(bool show)
{
    if (show)
    {
        ui->red_point->show();
    }
    else
    {
        ui->red_point->hide();
    }
}

std::shared_ptr<FriendListEntry> ConUserItem::getFriendEntry() const
{
    return _entry;
}
