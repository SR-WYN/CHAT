#include "ChatPage.h"
#include "AnimatedStateWidget.h"
#include "TcpMgr.h"
#include "global.h"
#include "ui_ChatPage.h"
#include <QPainter>
#include <QStyleOption>
#include "MessageTextEdit.h"
#include "ChatItemBase.h"
#include "TextBubble.h"
#include "PictureBubble.h"
#include "UserData.h"
#include "UserMgr.h"
#include <QJsonDocument>
#include <qjsonarray.h>
#include <qjsonobject.h>

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
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked()
{
    if (_user_info == nullptr)
    {
        return;
    }
    auto self_info = UserMgr::getInstance().getUserInfo();
    if (self_info == nullptr)
    {
        return;
    }
    auto peer_info = _user_info;
    auto pTextEdit = ui->chat_edit;
    ChatRole role = ChatRole::SELF;
    QString userName = self_info->_name;
    QString userIcon = self_info->_icon;

    const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    QJsonObject text_obj;
    QJsonArray text_array;
    int text_size = 0;
    for(int i=0; i<msgList.size(); ++i)
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
                text_obj["fromuid"] = self_info->_uid;
                text_obj["touid"] = peer_info->_uid;
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
            auto text_msg = std::make_shared<TextChatData>(uuid_string, obj["content"].toString(),
                                                           self_info->_uid, peer_info->_uid);
            emit sig_append_send_chat_msg(text_msg);
        }
        else if (type == "image")
        {
            pBubble = new PictureBubble(QPixmap(msgList[i].content),role);
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
    // 
    // 发送给服务器
    text_obj["text_array"] = text_array;
    text_obj["fromuid"] = self_info->_uid;
    text_obj["touid"] = peer_info->_uid;
    QJsonDocument doc(text_obj);
    QByteArray json_data = doc.toJson(QJsonDocument::Compact);
    // 发送并清空之前累积的文本列表
    text_size = 0;
    text_array = QJsonArray();
    text_obj = QJsonObject();
    // 发送tcp请求给 chatserver
    emit TcpMgr::getInstance().sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, json_data);
}

void ChatPage::setUserInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    ui->title_label->setText(_user_info->_name);
    ui->chat_data_list->removeAllItem();
    for (auto &msg:_user_info->_chat_msgs)
    {
        appendChatMsg(msg);
    }
}

void ChatPage::appendChatMsg(std::shared_ptr<TextChatData> msg)
{
    auto self_info = UserMgr::getInstance().getUserInfo();
    ChatRole role;
    if (msg->_from_uid == self_info->_uid)
    {
        role = ChatRole::SELF;
        ChatItemBase *pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->_name);
        pChatItem->setUserIcon(QPixmap(self_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role,msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
    else 
    {
        role = ChatRole::OTHER;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::getInstance().getFriendById(msg->_from_uid);
        if (friend_info == nullptr)
        {
            return;
        }
        pChatItem->setUserName(friend_info->_name);
        pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role,msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
}