#include "SearchList.h"
#include "AddUserItem.h"
#include "CustomsizeEdit.h"
#include "FriendRequestDialog.h"
#include "ListItemBase.h"
#include "Log.h"
#include "LogModule.h"
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
    this->viewport()->installEventFilter(this);
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    addTipItem();
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

        return true;
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
    item_tmp->setSizeHint(QSize(250, 10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);

    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}

void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item);
    if (!widget)
    {
        return;
    }

    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem)
    {
        return;
    }

    auto item_type = customItem->getItemType();
    if (item_type == ListItemType::INVALID_ITEM)
    {
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
        LOGI(LogModule::Ui, "search user uid={}", uid_str.toStdString());
        QJsonObject json_obj;
        json_obj["uid"] = uid_str;
        QJsonDocument doc(json_obj);
        QByteArray json_data = doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::getInstance().sig_send_data(ReqId::ID_SEARCH_USER_REQ, QString(json_data));
        return;
    }

    closeFindDlg();
}

void SearchList::slot_user_search(std::shared_ptr<UserProfile> profile)
{
    waitPending(false);
    closeFindDlg();
    if (profile == nullptr)
    {
        LOGW(LogModule::Ui, "search user not found");
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
            LOGW(LogModule::Ui, "search user is self uid={}", self_uid);
            return;
        }
        bool b_exist = UserMgr::getInstance().checkFriendById(profile->uid);
        if (b_exist)
        {
            LOGI(LogModule::Ui, "search user already friend uid={}", profile->uid);
            emit sig_jump_chat_item(profile);
            return;
        }
        LOGI(LogModule::Ui, "search user found uid={} name={}", profile->uid,
             profile->loginName.toStdString());
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
