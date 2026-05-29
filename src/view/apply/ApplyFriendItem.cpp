#include "ApplyFriendItem.h"
#include "UserModels.h"
#include "AnimatedStateWidget.h"
#include "ui_ApplyFriendItem.h"

ApplyFriendItem::ApplyFriendItem(QWidget *parent)
    : ListItemBase(parent), _added(false), ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::APPLY_FRIEND_ITEM);
    ui->add_btn->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->add_btn->setState("normal", "hover", "press");
    ui->add_btn->hide();
    connect(ui->add_btn, &AnimatedStateWidget::clicked, [this]() {
        emit this->sig_auth_friend(_apply_row);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(std::shared_ptr<PendingFriendApplyRow> apply_row)
{
    _apply_row = std::move(apply_row);
    QString icon = _apply_row->profile.icon;
    if (icon.isEmpty())
    {
        icon = QStringLiteral(":/res/head_1.png");
    }
    QPixmap pixmap(icon);
    ui->icon_label->setPixmap(
        pixmap.scaled(ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);
    ui->user_name_label->setText(_apply_row->profile.loginName);
    ui->user_chat_label->setText(_apply_row->profile.bio);
}

void ApplyFriendItem::ShowAddBtn(bool bshow)
{
    if (bshow)
    {
        ui->add_btn->show();
        ui->already_label->hide();
        _added = false;
    }
    else
    {
        ui->add_btn->hide();
        ui->already_label->show();
        _added = true;
    }
}

int ApplyFriendItem::GetUid()
{
    return _apply_row ? _apply_row->profile.uid : 0;
}
