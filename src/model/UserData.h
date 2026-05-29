#pragma once

#include <memory>
#include <vector>

#include <QJsonArray>
#include <QString>

struct TextChatData
{
    TextChatData(QString msg_id, QString msg_content, int fromuid, int touid)
        : _msg_id(msg_id), _msg_content(msg_content), _from_uid(fromuid), _to_uid(touid)
    {
    }
    QString _msg_id;
    QString _msg_content;
    int _from_uid;
    int _to_uid;
};

struct TextChatMsg
{
    TextChatMsg(int fromuid, int touid, QJsonArray arrays) : _from_uid(fromuid), _to_uid(touid)
    {
        for (auto msg_data : arrays)
        {
            auto msg_obj = msg_data.toObject();
            auto content = msg_obj["content"].toString();
            auto msgid = msg_obj["msgid"].toString();
            auto msg_ptr = std::make_shared<TextChatData>(msgid, content, fromuid, touid);
            _chat_msgs.push_back(msg_ptr);
        }
    }
    int _to_uid;
    int _from_uid;
    std::vector<std::shared_ptr<TextChatData>> _chat_msgs;
};
