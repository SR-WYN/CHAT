#include "ContactUserList.h"
#include "ConUserItem.h"
#include "GroupTipItem.h"
#include "TcpMgr.h"
#include "UserData.h"
#include "UserMgr.h"
#include <QEvent>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QWheelEvent>
#include <qdebug.h>
#include <qlistwidget.h>
#include <QTimer>

ContactUserList::ContactUserList(QWidget *parent):_add_friend_item(nullptr), _groupitem(nullptr), _load_pending(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    // 模拟从数据库或者后端传输过来的数据,进行列表加载
    addContactUserList();
    // 连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);
    // 链接对端同意认证后通知的信号
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_add_auth_friend, this,
            &ContactUserList::slot_add_auth_firend);
    // 链接自己点击同意认证后界面刷新
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_auth_rsp, this, &ContactUserList::slot_auth_rsp);
}
void ContactUserList::showRedPoint(bool bshow /*= true*/)
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
    _add_friend_item->setInfo(0, tr("新的朋友"), ":/res/add_friend.png");
    _add_friend_item->setItemType(ListItemType::APPLY_FRIEND_ITEM);
    QListWidgetItem *add_item = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    add_item->setSizeHint(_add_friend_item->sizeHint());
    this->addItem(add_item);
    this->setItemWidget(add_item, _add_friend_item);
    // 默认设置新的朋友申请条目被选中
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
        contact_user_widget->setInfo(contact_element->_uid, contact_element->_name,
                                     contact_element->_icon);
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(contact_user_widget->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, contact_user_widget);
    }
}
bool ContactUserList::eventFilter(QObject *watched, QEvent *event)
{
    // 检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport())
    {
        if (event->type() == QEvent::Enter)
        {
            // 鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        else if (event->type() == QEvent::Leave)
        {
            // 鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数
        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        // int pageSize = 10; // 每页加载的联系人数量
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
            qDebug() << "load more contact user";
            _load_pending = true;
            QTimer::singleShot(100, [this]() {
                _load_pending = false;
            });
            qDebug() << "emit loading contact user signal";
            emit sig_loading_contact_user();
        }
        return true; // 停止事件传递
    }
    return QListWidget::eventFilter(watched, event);
}
void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); // 获取自定义widget对象
    if (!widget)
    {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem)
    {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }
    auto itemType = customItem->getItemType();
    if (itemType == ListItemType::INVALID_ITEM || itemType == ListItemType::GROUP_TIP_ITEM)
    {
        qDebug() << "slot invalid item clicked ";
        return;
    }
    if (itemType == ListItemType::APPLY_FRIEND_ITEM)
    {
        // 创建对话框，提示用户
        qDebug() << "apply friend item clicked ";
        // 跳转到好友申请界面
        emit sig_switch_apply_friend_page();
        return;
    }
    if (itemType == ListItemType::CONTACT_USER_ITEM)
    {
        // 创建对话框，提示用户
        qDebug() << "contact user item clicked ";
        // 跳转到好友申请界面
        emit sig_switch_friend_info_page();
        return;
    }
}

void ContactUserList::slot_add_auth_firend(std::shared_ptr<AuthInfo> auth_info)
{
    qDebug() << "add auth friend signal received";
    bool is_friend = UserMgr::getInstance().checkFriendById(auth_info->_uid);
    if (is_friend)
    {
        qDebug() << auth_info->_name << " already is friend";
        return;
    }
    // 添加好友
    auto *con_user_widget = new ConUserItem;
    con_user_widget->setInfo(auth_info);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(con_user_widget->sizeHint());
    int index = this->row(_groupitem);
    this->insertItem(index + 1, item);
    this->setItemWidget(item, con_user_widget);
}
void ContactUserList::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "auth rsp signal received";
    bool is_friend = UserMgr::getInstance().checkFriendById(auth_rsp->_uid);
    if (is_friend)
    {
        qDebug() << auth_rsp->_name << " already is friend";
        return;
    }
    // 添加好友（头像使用服务端下发的 icon，与 ChatUserWidget 一致）
    auto *con_user_widget = new ConUserItem;
    con_user_widget->setInfo(auth_rsp);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(con_user_widget->sizeHint());
    int index = this->row(_groupitem);
    this->insertItem(index + 1, item);
    this->setItemWidget(item, con_user_widget);
}