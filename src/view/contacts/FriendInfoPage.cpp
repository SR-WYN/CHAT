#include "FriendInfoPage.h"
#include "AnimatedStateWidget.h"
#include "UserModels.h"
#include "ui_FriendInfoPage.h"
#include <QPixmap>
#include <QStringLiteral>
#include <memory>

FriendInfoPage::FriendInfoPage(QWidget *parent) : QWidget(parent), ui(new Ui::FriendInfoPage)
{
    ui->setupUi(this);
    ui->msg_chat_button->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->video_chat_button->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->voice_chat_button->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->msg_chat_button->setState("normal", "hover", "press");
    ui->video_chat_button->setState("normal", "hover", "press");
    ui->voice_chat_button->setState("normal", "hover", "press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;
}

void FriendInfoPage::setInfo(std::shared_ptr<FriendListEntry> entry)
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

    ui->name_tip->setText(_entry->profile.loginName);
    ui->alias_tip->setText(_entry->myAliasForPeer);
}

void FriendInfoPage::on_msg_chat_button_clicked()
{
    emit sig_jump_chat_item(_entry);
}
