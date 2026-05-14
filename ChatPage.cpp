#include "ChatPage.h"
#include "AnimatedStateWidget.h"
#include "ChatItemBase.h"
#include "MessageTextEdit.h"
#include "PictureBubble.h"
#include "TcpMgr.h"
#include "TextBubble.h"
#include "UserData.h"
#include "UserMgr.h"
#include "UserModels.h"
#include "global.h"
#include "ui_ChatPage.h"
#include <QPainter>
#include <QStyleOption>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ChatPage::ChatPage(QWidget *parent) : QWidget(parent), ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    ui->emoji_label->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->file_label->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->emoji_label->setState("normal", "hover", "press", "normal", "hover", "press");
    ui->file_label->setState("normal", "hover", "press", "normal", "hover", "press");
}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked()
{
    if (_peer == nullptr)
    {
        return;
    }
    auto self_info = UserMgr::getInstance().getSelfProfile();
    if (self_info == nullptr)
    {
        return;
    }
    auto peer_info = _peer;
    auto pTextEdit = ui->chat_edit;
    ChatRole role = ChatRole::SELF;
    QString userName = self_info->loginName;
    QString userIcon = self_info->icon;

    const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
    QJsonObject text_obj;
    QJsonArray text_array;
    int text_size = 0;
    for (int i = 0; i < msgList.size(); ++i)
    {
        if (msgList[i].content.length() > 1024)
        {
            qDebug() << "msg content length is too long, length:" << msgList[i].content.length();
            continue;
        }
        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;
        if (type == "text")
        {
            QUuid uuid = QUuid::createUuid();
            QString uuid_string = uuid.toString();
            pBubble = new TextBubble(role, msgList[i].content);
            if (text_size + msgList[i].content.length() > 1024)
            {
                text_obj["fromuid"] = self_info->uid;
                text_obj["touid"] = peer_info->uid();
                text_obj["text_array"] = text_array;
                QJsonDocument doc(text_obj);
                QByteArray json_data = doc.toJson(QJsonDocument::Compact);
                text_size = 0;
                text_array = QJsonArray();
                text_obj = QJsonObject();
                emit TcpMgr::getInstance().sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, json_data);
            }
            text_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgid"] = uuid_string;
            text_array.append(obj);
            auto text_msg = std::make_shared<TextChatData>(uuid_string, obj["content"].toString(), self_info->uid,
                                                          peer_info->uid());
            emit sig_append_send_chat_msg(text_msg);
        }
        else if (type == "image")
        {
            pBubble = new PictureBubble(QPixmap(msgList[i].content), role);
        }
        else if (type == "file")
        {
        }

        if (pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->chat_data_list->appendChatItem(pChatItem);
        }
    }
    qDebug() << "text_array is " << text_array;
    text_obj["text_array"] = text_array;
    text_obj["fromuid"] = self_info->uid;
    text_obj["touid"] = peer_info->uid();
    QJsonDocument doc(text_obj);
    QByteArray json_data = doc.toJson(QJsonDocument::Compact);
    text_size = 0;
    text_array = QJsonArray();
    text_obj = QJsonObject();
    emit TcpMgr::getInstance().sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, json_data);
}

void ChatPage::setFriendEntry(std::shared_ptr<FriendListEntry> peer)
{
    _peer = std::move(peer);
    ui->title_label->setText(_peer->listDisplayName());
    ui->chat_data_list->removeAllItem();
    for (const auto &msg : _peer->chat_msgs)
    {
        appendChatMsg(msg);
    }
}

void ChatPage::appendChatMsg(std::shared_ptr<TextChatData> msg)
{
    auto self_info = UserMgr::getInstance().getSelfProfile();
    if (!self_info)
    {
        return;
    }
    ChatRole role;
    if (msg->_from_uid == self_info->uid)
    {
        role = ChatRole::SELF;
        ChatItemBase *pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->loginName);
        pChatItem->setUserIcon(QPixmap(self_info->icon));
        QWidget *pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
    else
    {
        role = ChatRole::OTHER;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::getInstance().getFriendById(msg->_from_uid);
        QString showName = friend_info ? friend_info->listDisplayName() : QString();
        QString showIcon = friend_info ? friend_info->profile.icon : QString();
        if (showName.isEmpty() && _peer && _peer->uid() == msg->_from_uid)
        {
            showName = _peer->listDisplayName();
            showIcon = _peer->profile.icon;
        }
        if (showName.isEmpty())
        {
            return;
        }
        pChatItem->setUserName(showName);
        pChatItem->setUserIcon(QPixmap(showIcon));
        QWidget *pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
}
