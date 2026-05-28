#include "ContactUserList.h"
#include "ConUserItem.h"
#include "GroupTipItem.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "UserModels.h"
#include "global.h"
#include <QEvent>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QWheelEvent>
#include <qdebug.h>
#include <qlistwidget.h>
#include <QTimer>

ContactUserList::ContactUserList(QWidget *parent)
    : QListWidget(parent), _add_friend_item(nullptr), _groupitem(nullptr), _load_pending(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->viewport()->installEventFilter(this);
    addContactUserList();
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_add_auth_friend, this,
            &ContactUserList::slot_add_auth_firend);
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_auth_rsp, this, &ContactUserList::slot_auth_rsp);
}

void ContactUserList::showRedPoint(bool bshow)
{
    _add_friend_item->showRedPoint(bshow);
}

void ContactUserList::addContactUserList()
{
    auto *groupTip = new GroupTipItem();
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(groupTip->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, groupTip);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    _add_friend_item = new ConUserItem();
    _add_friend_item->setObjectName("new_friend_item");
    UserProfile np;
    np.uid = 0;
    np.loginName = tr("新的朋友");
    np.nick = np.loginName;
    np.icon = QStringLiteral(":/res/add_friend.png");
    np.sex = 0;
    _add_friend_item->setInfo(std::move(np));
    _add_friend_item->setItemType(ListItemType::APPLY_FRIEND_ITEM);
    QListWidgetItem *add_item = new QListWidgetItem;
    add_item->setSizeHint(_add_friend_item->sizeHint());
    this->addItem(add_item);
    this->setItemWidget(add_item, _add_friend_item);
    this->setCurrentItem(add_item);
    auto *groupCon = new GroupTipItem();
    groupCon->setGroupTip(tr("联系人"));
    _groupitem = new QListWidgetItem;
    _groupitem->setSizeHint(groupCon->sizeHint());
    this->addItem(_groupitem);
    this->setItemWidget(_groupitem, groupCon);
    _groupitem->setFlags(_groupitem->flags() & ~Qt::ItemIsSelectable);

    auto contact_list = UserMgr::getInstance().getContactListPerpage();
    for (auto &contact_element : contact_list)
    {
        auto *contact_user_widget = new ConUserItem();
        contact_user_widget->setInfo(contact_element->profile);
        QListWidgetItem *witem = new QListWidgetItem;
        witem->setSizeHint(contact_user_widget->sizeHint());
        this->addItem(witem);
        this->setItemWidget(witem, contact_user_widget);
    }
}

bool ContactUserList::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this->viewport())
    {
        if (event->type() == QEvent::Enter)
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        else if (event->type() == QEvent::Leave)
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
    if (watched == this->viewport() && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15;
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        if (maxScrollValue - currentValue <= 0)
        {
            auto b_loaded = UserMgr::getInstance().isLoadContactFinish();
            if (b_loaded)
            {
                return true;
            }
            if (_load_pending)
            {
                return true;
            }
            _load_pending = true;
            QTimer::singleShot(100, [this]() {
                _load_pending = false;
            });
            emit sig_loading_contact_user();
        }
        return true;
    }
    return QListWidget::eventFilter(watched, event);
}

void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item);
    if (!widget)
    {
        return;
    }
    ConUserItem *customItem = qobject_cast<ConUserItem *>(widget);
    if (!customItem)
    {
        return;
    }
    auto itemType = customItem->getItemType();
    if (itemType == ListItemType::INVALID_ITEM || itemType == ListItemType::GROUP_TIP_ITEM)
    {
        return;
    }
    if (itemType == ListItemType::APPLY_FRIEND_ITEM)
    {
        emit sig_switch_apply_friend_page();
        return;
    }
    if (itemType == ListItemType::CONTACT_USER_ITEM)
    {
        emit sig_switch_friend_info_page(customItem->getFriendEntry());
        return;
    }
}

void ContactUserList::slot_add_auth_firend(std::shared_ptr<AuthAcceptedPeer> peer)
{
    if (!peer)
    {
        return;
    }
    if (UserMgr::getInstance().checkFriendById(peer->profile.uid))
    {
        return;
    }
    auto *con_user_widget = new ConUserItem;
    con_user_widget->setInfo(peer);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(con_user_widget->sizeHint());
    int index = this->row(_groupitem);
    this->insertItem(index + 1, item);
    this->setItemWidget(item, con_user_widget);
}

void ContactUserList::slot_auth_rsp(std::shared_ptr<AuthAcceptedPeer> peer)
{
    if (!peer)
    {
        return;
    }
    if (UserMgr::getInstance().checkFriendById(peer->profile.uid))
    {
        return;
    }
    auto *con_user_widget = new ConUserItem;
    con_user_widget->setInfo(peer);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(con_user_widget->sizeHint());
    int index = this->row(_groupitem);
    this->insertItem(index + 1, item);
    this->setItemWidget(item, con_user_widget);
}
