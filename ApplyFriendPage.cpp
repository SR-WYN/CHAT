#include "ApplyFriendPage.h"
#include "ApplyFriendItem.h"
#include "FriendRequestDialog.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "UserModels.h"
#include "ui_ApplyFriendPage.h"
#include <QListWidgetItem>

void ApplyFriendPage::loadApplyList()
{
    auto apply_list = UserMgr::getInstance().getApplyList();
    for (auto &apply : apply_list)
    {
        auto *apply_item = new ApplyFriendItem();
        apply_item->SetInfo(apply.second);
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
        ui->apply_friend_list->insertItem(0, item);
        ui->apply_friend_list->setItemWidget(item, apply_item);
        if (apply.second->status)
        {
            apply_item->ShowAddBtn(false);
        }
        else
        {
            apply_item->ShowAddBtn(true);
        }
        connect(apply_item, &ApplyFriendItem::sig_auth_friend,
                [this](std::shared_ptr<PendingFriendApplyRow> apply_info) {
                    auto *authFriend = new FriendRequestDialog(this, FriendRequestDialog::Mode::Auth);
                    authFriend->setModal(true);
                    authFriend->setApplyInfo(apply_info);
                    authFriend->show();
                });
    }
}

ApplyFriendPage::ApplyFriendPage(QWidget *parent) : QWidget(parent), ui(new Ui::ApplyFriendPage)
{
    ui->setupUi(this);
    loadApplyList();
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_auth_rsp, this, &ApplyFriendPage::slot_auth_rsp);
}

ApplyFriendPage::~ApplyFriendPage()
{
    delete ui;
}

void ApplyFriendPage::addNewApply(std::shared_ptr<FriendApplyNotify> notify)
{
    if (!notify)
    {
        return;
    }

    auto info = UserMgr::getInstance().getApply(notify->applicant.uid);
    if (!info)
    {
        return;
    }

    auto *apply_item = new ApplyFriendItem();
    apply_item->SetInfo(info);
    apply_item->ShowAddBtn(true);

    auto *lwItem = new QListWidgetItem;
    lwItem->setSizeHint(apply_item->sizeHint());
    ui->apply_friend_list->insertItem(0, lwItem);
    ui->apply_friend_list->setItemWidget(lwItem, apply_item);
    connect(apply_item, &ApplyFriendItem::sig_auth_friend,
            [this](std::shared_ptr<PendingFriendApplyRow> apply_info) {
                auto *authFriend = new FriendRequestDialog(this, FriendRequestDialog::Mode::Auth);
                authFriend->setModal(true);
                authFriend->setApplyInfo(apply_info);
                authFriend->show();
            });
}

void ApplyFriendPage::slot_auth_rsp(std::shared_ptr<AuthAcceptedPeer> peer)
{
    if (!peer)
    {
        return;
    }
    const int uid = peer->profile.uid;
    if (auto info = UserMgr::getInstance().getApply(uid))
    {
        info->status = 1;
    }
    for (int i = 0; i < ui->apply_friend_list->count(); ++i)
    {
        QListWidgetItem *lw_item = ui->apply_friend_list->item(i);
        if (!lw_item)
        {
            continue;
        }
        auto *apply_item = qobject_cast<ApplyFriendItem *>(ui->apply_friend_list->itemWidget(lw_item));
        if (!apply_item || apply_item->GetUid() != uid)
        {
            continue;
        }
        apply_item->ShowAddBtn(false);
        break;
    }
}
