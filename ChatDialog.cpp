#include "ChatDialog.h"
#include "ChatPage.h"
#include "ChatUserList.h"
#include "ChatUserWidget.h"
#include "ConUserItem.h"
#include "ContactUserList.h"
#include "FriendInfoPage.h"
#include "ListItemBase.h"
#include "QAction"
#include "SearchList.h"
#include "StatusDialog.h"
#include "TcpMgr.h"
#include "UserData.h"
#include "UserMgr.h"
#include "UserModels.h"
#include "global.h"
#include "ui_ChatDialog.h"
#include <QMouseEvent>
#include <QRandomGenerator>
#include <memory>
#include <qaction.h>
#include <qdebug.h>
#include <qicon.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qobject.h>
#include <qstringliteral.h>
#include <qwidget.h>
#include <unistd.h>

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatDialog), _mode(ChatUIMode::CHAT_MODE),
      _state(ChatUIMode::CHAT_MODE), _b_loading(false), _cur_chat_uid(0), _last_widget(nullptr)
{
    ui->setupUi(this);
    ui->add_btn->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->add_btn->setState("normal", "hover", "press");
    ui->search_edit->setMaxLength(15);

    QAction *search_action = new QAction(ui->search_edit);
    search_action->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(search_action, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    QAction *clear_action = new QAction(ui->search_edit);
    clear_action->setIcon(QIcon(":/res/empty.png"));
    ui->search_edit->addAction(clear_action, QLineEdit::TrailingPosition);

    connect(ui->search_edit, &QLineEdit::textChanged, [clear_action](const QString &text) {
        if (text.isEmpty())
        {
            clear_action->setIcon(QIcon(":/res/empty.png"));
        }
        else
        {
            clear_action->setIcon(QIcon(":/res/close_search.png"));
        }
    });

    connect(clear_action, &QAction::triggered, [this, clear_action]() {
        ui->search_edit->clear();
        clear_action->setIcon(QIcon(":/res/empty.png"));
        ui->search_edit->clearFocus();
        showSearch(false);
    });

    showSearch(false);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this,
            &ChatDialog::slot_loading_chat_user);
    addChatUserList();

    addLabelGroup(ui->side_chat_label);
    addLabelGroup(ui->side_contact_label);
    ui->side_chat_label->setSelected(true);
    ui->side_contact_label->setSelected(false);

    connect(ui->side_chat_label, &AnimatedStateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_label, &AnimatedStateWidget::clicked, this,
            &ChatDialog::slot_side_contact);

    connect(ui->con_user_list, &ContactUserList::sig_switch_apply_friend_page, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    });

    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);
    ui->search_list->setSearchEdit(ui->search_edit);

    this->installEventFilter(this);

    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_friend_apply, this,
            &ChatDialog::slot_friend_apply);
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_add_auth_friend, this,
            &ChatDialog::slot_add_auth_friend);
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_auth_rsp, this, &ChatDialog::slot_auth_rsp);
    connect(ui->search_list, &SearchList::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);
    connect(ui->con_user_list, &ContactUserList::sig_loading_contact_user, this,
            &ChatDialog::slot_loading_contact_user);
    connect(ui->con_user_list, &ContactUserList::sig_switch_friend_info_page, this,
            &ChatDialog::slot_friend_info_page);
    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item, this,
            &ChatDialog::slot_jump_chat_item_from_infopage);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    connect(ui->chat_user_list, &QListWidget::itemClicked, this, &ChatDialog::slot_item_clicked);

    setSelectChatItem();
    setSelectChatPage();

    connect(ui->chat_page, &ChatPage::sig_append_send_chat_msg, this,
            &ChatDialog::slot_append_send_chat_msg);

    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);
    connect(TcpMgr::getInstancePtr(), &TcpMgr::sig_chat_history, this, &ChatDialog::slot_chat_history);
    refreshChatListFromMemory();
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::showSearch(bool bsearch)
{
    if (bsearch)
    {
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SEARCH_MODE;
    }
    else if (_state == ChatUIMode::CHAT_MODE)
    {
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_list->hide();
        _mode = ChatUIMode::CHAT_MODE;
    }
    else if (_state == ChatUIMode::CONTACT_MODE)
    {
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::CONTACT_MODE;
    }
}

void ChatDialog::addChatUserList()
{
    auto friend_list = UserMgr::getInstance().getChatListPerpage();
    if (!friend_list.empty())
    {
        for (auto &friend_element : friend_list)
        {
            auto find_iter = _chat_item_added.find(friend_element->uid());
            if (find_iter != _chat_item_added.end())
            {
                continue;
            }
            auto *chat_user_widget = new ChatUserWidget();
            chat_user_widget->setInfo(friend_element);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(chat_user_widget->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_widget);
            _chat_item_added.insert(friend_element->uid(), item);
        }
        UserMgr::getInstance().updateChatLoadedCount();
    }
}

void ChatDialog::slot_loading_chat_user()
{
    if (_b_loading)
    {
        return;
    }
    _b_loading = true;
    StatusDialog *loading_dialog = new StatusDialog(this);
    loading_dialog->setMode(StatusDialog::StatusMode::Loading);
    loading_dialog->setModal(true);
    loading_dialog->show();
    qDebug() << "add new data to list...";
    loadMoreChatUser();
    loading_dialog->deleteLater();
    _b_loading = false;
}

void ChatDialog::addLabelGroup(AnimatedStateWidget *label)
{
    _label_list.push_back(label);
}

void ChatDialog::slot_side_chat()
{
    qDebug() << "receive side chat clicked";
    clearLabelState(ui->side_chat_label);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::CHAT_MODE;
    showSearch(false);
}

void ChatDialog::slot_side_contact()
{
    qDebug() << "receive side contact clicked";
    clearLabelState(ui->side_contact_label);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    _state = ChatUIMode::CONTACT_MODE;
    showSearch(false);
}

void ChatDialog::clearLabelState(AnimatedStateWidget *active)
{
    for (auto *item : _label_list)
    {
        item->setSelected(item == active);
    }
}

void ChatDialog::slot_text_changed(const QString &str)
{
    if (!str.isEmpty())
    {
        showSearch(true);
    }
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        handleGlobalMousePress(mouse_event);
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *mouse_event)
{
    if (_mode != ChatUIMode::SEARCH_MODE)
    {
        return;
    }

    QPoint posInSearchList =
        ui->search_list->mapFromGlobal(mouse_event->globalPosition().toPoint());
    if (!ui->search_list->rect().contains(posInSearchList))
    {
        ui->search_edit->clear();
        showSearch(false);
    }
}

void ChatDialog::slot_friend_apply(std::shared_ptr<FriendApplyNotify> apply)
{
    qDebug() << "receive friend apply, uid is " << apply->applicant.uid;
    if (UserMgr::getInstance().alreadyApply(apply->applicant.uid))
    {
        qDebug() << "already apply, return";
        return;
    }
    UserMgr::getInstance().addApply(std::make_shared<PendingFriendApplyRow>(*apply));
    ui->side_contact_label->showRedPoint(true);
    ui->con_user_list->showRedPoint(true);
    ui->friend_apply_page->addNewApply(apply);
}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthAcceptedPeer> peer)
{
    qDebug() << "receive slot_add_auth_friend uid is " << peer->profile.uid;
    if (UserMgr::getInstance().checkFriendById(peer->profile.uid))
    {
        qDebug() << peer->profile.loginName << " already is friend";
        return;
    }
    UserMgr::getInstance().addFriend(peer);
    auto *chat_user_widget = new ChatUserWidget;
    chat_user_widget->setInfo(UserMgr::getInstance().getFriendById(peer->profile.uid));
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_widget->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_widget);
    _chat_item_added.insert(peer->profile.uid, item);
}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthAcceptedPeer> peer)
{
    qDebug() << "receive slot_auth_rsp uid is " << peer->profile.uid;
    if (UserMgr::getInstance().checkFriendById(peer->profile.uid))
    {
        qDebug() << peer->profile.loginName << " already is friend";
        return;
    }
    UserMgr::getInstance().addFriend(peer);
    auto *chat_user_widget = new ChatUserWidget;
    chat_user_widget->setInfo(UserMgr::getInstance().getFriendById(peer->profile.uid));
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_widget->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_widget);
    _chat_item_added.insert(peer->profile.uid, item);
}

void ChatDialog::slot_jump_chat_item(std::shared_ptr<UserProfile> profile)
{
    qDebug() << "slot jump chat item";
    if (!profile)
    {
        return;
    }
    auto find_iter = _chat_item_added.find(profile->uid);
    if (find_iter == _chat_item_added.end())
    {
        auto entry = std::make_shared<FriendListEntry>(*profile);
        auto *chat_user_widget = new ChatUserWidget;
        chat_user_widget->setInfo(entry);
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(chat_user_widget->sizeHint());
        ui->chat_user_list->insertItem(0, item);
        ui->chat_user_list->setItemWidget(item, chat_user_widget);
        _chat_item_added.insert(profile->uid, item);
        return;
    }
    qDebug() << "jump chat item,uid is " << profile->uid;
    ui->chat_user_list->scrollToItem(find_iter.value());
    clearLabelState(ui->side_chat_label);
    setSelectChatItem(profile->uid);
    setSelectChatPage(profile->uid);
}

void ChatDialog::setSelectChatItem(int uid)
{
    if (ui->chat_user_list->count() <= 0)
    {
        return;
    }
    if (uid == 0)
    {
        ui->chat_user_list->setCurrentRow(0);
        QListWidgetItem *first_item = ui->chat_user_list->item(0);
        if (!first_item)
        {
            return;
        }

        QWidget *widget = ui->chat_user_list->itemWidget(first_item);
        if (!widget)
        {
            return;
        }

        auto con_item = qobject_cast<ChatUserWidget *>(widget);
        if (!con_item)
        {
            return;
        }

        auto fe = con_item->getFriendEntry();
        _cur_chat_uid = fe ? fe->uid() : 0;
        return;
    }
    auto find_iter = _chat_item_added.find(uid);
    if (find_iter == _chat_item_added.end())
    {
        qDebug() << "uid:" << uid << " not found";
        ui->chat_user_list->setCurrentRow(0);
        return;
    }
    ui->chat_user_list->setCurrentItem(find_iter.value());
    _cur_chat_uid = uid;
}

void ChatDialog::setSelectChatPage(int uid)
{
    if (ui->chat_user_list->count() <= 0)
    {
        return;
    }
    if (uid == 0)
    {
        auto item = ui->chat_user_list->item(0);
        QWidget *widget = ui->chat_user_list->itemWidget(item);
        if (!widget)
        {
            return;
        }
        auto con_item = qobject_cast<ChatUserWidget *>(widget);
        if (!con_item)
        {
            return;
        }
        auto entry = con_item->getFriendEntry();
        ui->chat_page->setFriendEntry(entry);
        if (entry)
        {
            requestChatHistory(entry->uid());
        }
        return;
    }
    auto find_iter = _chat_item_added.find(uid);
    if (find_iter == _chat_item_added.end())
    {
        return;
    }
    QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget)
    {
        return;
    }
    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem)
    {
        qDebug() << "slot setSelectChatPage customItem is nullptr";
        return;
    }
    auto itemType = customItem->getItemType();
    if (itemType == CHAT_USER_ITEM)
    {
        auto con_item = qobject_cast<ChatUserWidget *>(customItem);
        if (!con_item)
        {
            return;
        }
        auto entry = con_item->getFriendEntry();
        ui->chat_page->setFriendEntry(entry);
        if (entry)
        {
            requestChatHistory(entry->uid());
        }
        return;
    }
}

void ChatDialog::requestChatHistory(int peer_uid)
{
    if (peer_uid <= 0)
    {
        return;
    }
    TcpMgr::getInstance().requestChatHistory(peer_uid, 0, 100);
}

void ChatDialog::slot_chat_history(int peer_uid, std::vector<std::shared_ptr<TextChatData>> msgs)
{
    UserMgr::getInstance().mergeFriendChatHistory(peer_uid, msgs);
    refreshChatListItem(peer_uid);
    if (_cur_chat_uid != peer_uid)
    {
        return;
    }
    const auto entry = UserMgr::getInstance().getFriendById(peer_uid);
    if (!entry)
    {
        return;
    }
    ui->chat_page->setFriendEntry(entry);
}

void ChatDialog::refreshChatListFromMemory()
{
    for (auto it = _chat_item_added.begin(); it != _chat_item_added.end(); ++it)
    {
        refreshChatListItem(it.key());
    }
}

void ChatDialog::refreshChatListItem(int peer_uid)
{
    const auto find_iter = _chat_item_added.find(peer_uid);
    if (find_iter == _chat_item_added.end())
    {
        return;
    }
    const auto entry = UserMgr::getInstance().getFriendById(peer_uid);
    if (!entry)
    {
        return;
    }
    QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
    auto *chat_widget = qobject_cast<ChatUserWidget *>(widget);
    if (chat_widget)
    {
        chat_widget->setInfo(entry);
    }
}

void ChatDialog::loadMoreChatUser()
{
    auto friend_list = UserMgr::getInstance().getChatListPerpage();
    if (!friend_list.empty())
    {
        for (auto &friend_element : friend_list)
        {
            auto find_iter = _chat_item_added.find(friend_element->uid());
            if (find_iter != _chat_item_added.end())
            {
                continue;
            }
            auto *chat_user_widget = new ChatUserWidget();
            chat_user_widget->setInfo(friend_element);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(chat_user_widget->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_widget);
            _chat_item_added.insert(friend_element->uid(), item);
        }
        UserMgr::getInstance().updateChatLoadedCount();
    }
}

void ChatDialog::loadMoreContactUser()
{
    auto contact_list = UserMgr::getInstance().getContactListPerpage();
    if (!contact_list.empty())
    {
        for (auto &contact_element : contact_list)
        {
            auto *chat_user_widget = new ConUserItem();
            chat_user_widget->setInfo(contact_element->profile);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(chat_user_widget->sizeHint());
            ui->con_user_list->addItem(item);
            ui->con_user_list->setItemWidget(item, chat_user_widget);
        }
        UserMgr::getInstance().updateContactLoadedCount();
    }
}

void ChatDialog::slot_loading_contact_user()
{
    qDebug() << "slot loading contact user";
    if (_b_loading)
    {
        return;
    }
    _b_loading = true;
    StatusDialog *loading_dialog = new StatusDialog(this);
    loading_dialog->setMode(StatusDialog::StatusMode::Loading);
    loading_dialog->setModal(true);
    loading_dialog->show();
    qDebug() << "start loading contact user";
    loadMoreContactUser();
    loading_dialog->deleteLater();
    _b_loading = false;
}

void ChatDialog::slot_friend_info_page(std::shared_ptr<FriendListEntry> entry)
{
    qDebug() << "receive switch friend info page sig";
    _last_widget = ui->friend_info_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    ui->friend_info_page->setInfo(entry);
}

void ChatDialog::slot_jump_chat_item_from_infopage(std::shared_ptr<FriendListEntry> entry)
{
    qDebug() << "receive jump chat item from infopage sig";
    if (!entry)
    {
        return;
    }
    const int uid = entry->uid();
    auto find_iter = _chat_item_added.find(uid);
    if (find_iter != _chat_item_added.end())
    {
        qDebug() << "jump to chat item , uid is " << uid;
        ui->chat_user_list->scrollToItem(find_iter.value());
        clearLabelState(ui->side_chat_label);
        setSelectChatItem(uid);
        setSelectChatPage(uid);
        slot_side_chat();
        return;
    }
    auto *chat_user_widget = new ChatUserWidget();
    chat_user_widget->setInfo(entry);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_widget->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_widget);
    _chat_item_added.insert(uid, item);
    ui->chat_user_list->scrollToItem(item);
    clearLabelState(ui->side_chat_label);
    setSelectChatItem(uid);
    setSelectChatPage(uid);
    slot_side_chat();
}

void ChatDialog::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = ui->chat_user_list->itemWidget(item);
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
    auto itemType = customItem->getItemType();
    if (itemType == ListItemType::INVALID_ITEM || itemType == ListItemType::GROUP_TIP_ITEM)
    {
        qDebug() << "slot invalid item clicked ";
        return;
    }
    if (itemType == ListItemType::CHAT_USER_ITEM)
    {
        qDebug() << "slot chat user item clicked ";
        auto chat_widget = qobject_cast<ChatUserWidget *>(customItem);
        auto entry = chat_widget->getFriendEntry();
        ui->chat_page->setFriendEntry(entry);
        _cur_chat_uid = entry ? entry->uid() : 0;
        return;
    }
}

void ChatDialog::slot_append_send_chat_msg(std::shared_ptr<TextChatData> msg)
{
    if (_cur_chat_uid == 0)
    {
        return;
    }
    auto find_iter = _chat_item_added.find(_cur_chat_uid);
    if (find_iter == _chat_item_added.end())
    {
        return;
    }
    QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget)
    {
        return;
    }
    auto customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem)
    {
        qDebug() << "slot append send chat msg customItem is nullptr";
        return;
    }
    auto itemType = customItem->getItemType();
    if (itemType == ListItemType::CHAT_USER_ITEM)
    {
        auto con_item = qobject_cast<ChatUserWidget *>(customItem);
        if (!con_item)
        {
            return;
        }
        auto entry = con_item->getFriendEntry();
        if (!entry)
        {
            return;
        }
        std::vector<std::shared_ptr<TextChatData>> msg_vec;
        msg_vec.push_back(msg);
        entry->appendChatMsgs(msg_vec);
        UserMgr::getInstance().appendFriendChatMsg(_cur_chat_uid, msg_vec);
        ui->chat_page->appendChatMsg(msg);
    }
}

void ChatDialog::slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg_ptr)
{
    qDebug() << "receive slot_text_chat_msg";
    auto find_iter = _chat_item_added.find(msg_ptr->_from_uid);
    if (find_iter != _chat_item_added.end())
    {
        QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
        auto chat_widget = qobject_cast<ChatUserWidget *>(widget);
        if (!chat_widget)
        {
            return;
        }
        refreshChatListItem(msg_ptr->_from_uid);
        updateChatMsg(msg_ptr->_chat_msgs);
        return;
    }

    auto friend_info = UserMgr::getInstance().getFriendById(msg_ptr->_from_uid);
    if (!friend_info)
    {
        qDebug() << "slot_text_chat_msg: no friend row for uid" << msg_ptr->_from_uid;
        return;
    }
    auto *chat_user_widget = new ChatUserWidget();
    chat_user_widget->setInfo(friend_info);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_widget->sizeHint());
    refreshChatListItem(msg_ptr->_from_uid);
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_widget);
    _chat_item_added.insert(msg_ptr->_from_uid, item);
}

void ChatDialog::updateChatMsg(const std::vector<std::shared_ptr<TextChatData>> &msg_vec)
{
    const int self_uid = UserMgr::getInstance().getUid();
    for (auto &msg : msg_vec)
    {
        const int peer_uid =
            (msg->_from_uid == self_uid) ? msg->_to_uid : msg->_from_uid;
        if (peer_uid != _cur_chat_uid)
        {
            continue;
        }
        ui->chat_page->appendChatMsg(msg);
    }
}
