#include "SearchList.h"
#include "AddUserItem.h"
#include "FindSuccessDialog.h"
#include "ListItemBase.h"
#include "LoadingDialog.h"
#include "TcpMgr.h"
#include "UserData.h"
#include "global.h"

SearchList::SearchList(QWidget *parent)
    : QListWidget(parent), _find_dlg(nullptr), _search_edit(nullptr), _send_pending(false)
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
        _find_dlg = nullptr;
    }
}

void SearchList::setSearchEdit(QWidget *edit)
{
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
        // if (_send_pending)
        // {
        //     return;
        // }
        // waitPending(true);
        // auto search_edit = dynamic_cast<CustomizeEdit*>(_search_edit);
        // auto uid_str = search_edit->text();

        // QJsonObject json_obj;
        // json_obj["uid"] = uid_str;
        // QJsonDocument doc(json_obj);
        // QString json_string = doc.toJson(QJsonDocument::Indented);
        // emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,json_string);
        // todo ...
        _find_dlg = std::make_shared<FindSuccessDialog>(this);
        auto si = std::make_shared<SearchInfo>(0, "wyn", "wyn", "hello world!!!!!!!!", 0);
        (std::dynamic_pointer_cast<FindSuccessDialog>(_find_dlg))->setSearchInfo(si);
        _find_dlg->show();
        return;
    }

    // 清除弹出框
    closeFindDlg();
}

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{
}