#pragma once

#include <memory>
#include <vector>

#include <QJsonArray>
#include <QString>

// 消息类型：0=文本，1=图片
enum class ChatMsgType
{
    Text = 0,
    Image = 1
};

struct TextChatData
{
    TextChatData(QString msg_id, QString msg_content, int fromuid, int touid,
                 ChatMsgType msg_type = ChatMsgType::Text, QString url = QString())
        : _msg_id(msg_id),
          _msg_content(msg_content),
          _from_uid(fromuid),
          _to_uid(touid),
          _msg_type(msg_type),
          _url(url)
    {
    }

    QString _msg_id;
    QString _msg_content;
    int _from_uid;
    int _to_uid;
    ChatMsgType _msg_type = ChatMsgType::Text;
    QString _url; // 图片/文件消息的 URL
};

struct TextChatMsg
{
    TextChatMsg(int fromuid, int touid, QJsonArray arrays)
        : _from_uid(fromuid), _to_uid(touid)
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

struct ImageChatData
{
    ImageChatData(QString msg_id, QString url, int width, int height, qint64 size,
                  QString filename, int fromuid, int touid)
        : _msg_id(msg_id),
          _url(url),
          _width(width),
          _height(height),
          _size(size),
          _filename(filename),
          _from_uid(fromuid),
          _to_uid(touid)
    {
    }

    QString _msg_id;
    QString _url;
    int _width = 0;
    int _height = 0;
    qint64 _size = 0;
    QString _filename;
    int _from_uid = 0;
    int _to_uid = 0;
};

struct ImageChatMsg
{
    ImageChatMsg(int fromuid, int touid, QJsonArray arrays)
        : _from_uid(fromuid), _to_uid(touid)
    {
        for (auto msg_data : arrays)
        {
            auto msg_obj = msg_data.toObject();
            auto msgid = msg_obj["msgid"].toString();
            auto url = msg_obj["url"].toString();
            auto width = msg_obj["width"].toInt();
            auto height = msg_obj["height"].toInt();
            auto size = msg_obj["size"].toVariant().toLongLong();
            auto filename = msg_obj["filename"].toString();
            auto msg_ptr = std::make_shared<ImageChatData>(
                msgid, url, width, height, size, filename, fromuid, touid);
            _chat_msgs.push_back(msg_ptr);
        }
    }
    int _to_uid;
    int _from_uid;
    std::vector<std::shared_ptr<ImageChatData>> _chat_msgs;
};
