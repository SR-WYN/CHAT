#include "SearchList.h"
#include "AddUserItem.h"
#include "CustomsizeEdit.h"
#include "FriendRequestDialog.h"
#include "ListItemBase.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "UserModels.h"
#include "global.h"
#include <QDebug>
#include <QLineEdit>
#include <memory>
#include <qjsondocument.h>

SearchList::SearchList(QWidget *parent)
    : QListWidget(parent), _find_dlg(nullptr), _search_edit(nullptr), _loadingDialog(nullptr),
      _send_pending(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    // 连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    // 添加条目
    addTipItem();
    // 连接搜索条目
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_user_search, this,
            &SearchList::slot_user_search);
}

void SearchList::closeFindDlg()
{
    if (_find_dlg)
    {
        _find_dlg->hide();
        delete _find_dlg;
        _find_dlg = nullptr;
    }
}

void SearchList::setSearchEdit(QWidget *edit)
{
    _search_edit = edit;
}

bool SearchList::eventFilter(QObject *watched, QEvent *event)
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

        return true; // 停止事件传递
    }

    return QListWidget::eventFilter(watched, event);
}

void SearchList::waitPending(bool pending)
{
    if (pending)
    {
        _loadingDialog = new StatusDialog(this);
        _loadingDialog->setMode(StatusDialog::StatusMode::Loading);
        _loadingDialog->setModal(true);
        _loadingDialog->show();
        _send_pending = true;
    }
    else
    {
        _loadingDialog->hide();
        _loadingDialog->deleteLater();
        _send_pending = false;
    }
}

void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250, 10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);

    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}

void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item);
    if (!widget)
    {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem)
    {
        qDebug() << "slot item clicked customItem is nullptr";
        return;
    }

    auto item_type = customItem->getItemType();
    if (item_type == ListItemType::INVALID_ITEM)
    {
        qDebug() << "slot invalid item clicked";
        return;
    }

    if (item_type == ListItemType::ADD_TIP_USER_ITEM)
    {
        if (_send_pending)
        {
            return;
        }
        waitPending(true);
        auto search_edit = dynamic_cast<CustomsizeEdit *>(_search_edit);
        auto uid_str = search_edit->text();
        QJsonObject json_obj;
        json_obj["uid"] = uid_str;
        QJsonDocument doc(json_obj);
        QByteArray json_data = doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::getInstance().sig_send_data(ReqId::ID_SEARCH_USER_REQ, QString(json_data));
        return;
    }

    // 清除弹出框
    closeFindDlg();
}

void SearchList::slot_user_search(std::shared_ptr<UserProfile> profile)
{
    waitPending(false);
    closeFindDlg();
    if (profile == nullptr)
    {
        qDebug() << "slot_user_search profile is nullptr";
        _find_dlg = new StatusDialog(this);
        _find_dlg->setMode(StatusDialog::StatusMode::Fail);
        _find_dlg->show();
        return;
    }
    else
    {
        auto self_uid = UserMgr::getInstance().getUid();
        if (self_uid == profile->uid)
        {
            qDebug() << "don't search myself";
            return;
        }
        bool b_exist = UserMgr::getInstance().checkFriendById(profile->uid);
        if (b_exist)
        {
            emit sig_jump_chat_item(profile);
            return;
        }
        _find_dlg = new StatusDialog(this);
        _find_dlg->setMode(StatusDialog::StatusMode::Success);
        _find_dlg->setSearchInfo(profile);
        connect(_find_dlg, &StatusDialog::sig_add_friend, this,
                [this](std::shared_ptr<UserProfile> p) {
                    auto *apply_dlg = new FriendRequestDialog(this, FriendRequestDialog::Mode::Apply);
                    apply_dlg->setSearchInfo(p);
                    apply_dlg->show();
                });
    }
    _find_dlg->show();
}