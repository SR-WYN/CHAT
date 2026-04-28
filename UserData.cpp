#include "UserData.h"
#include <QJsonArray>
#include <QJsonObject>

// SearchInfo 实现
SearchInfo::SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon)
    : _uid(uid), _name(name), _nick(nick), _desc(desc), _sex(sex), _icon(icon)
{
}

int SearchInfo::getUid() const
{
    return _uid;
}

QString SearchInfo::getName() const
{
    return _name;
}

QString SearchInfo::getNick() const
{
    return _nick;
}

QString SearchInfo::getDesc() const
{
    return _desc;
}

int SearchInfo::getSex() const
{
    return _sex;
}

QString SearchInfo::getIcon() const
{
    return _icon;
}

void SearchInfo::setUid(int uid)
{
    _uid = uid;
}

void SearchInfo::setName(QString name)
{
    _name = name;
}

void SearchInfo::setNick(QString nick)
{
    _nick = nick;
}

void SearchInfo::setDesc(QString desc)
{
    _desc = desc;
}

void SearchInfo::setSex(int sex)
{
    _sex = sex;
}

void SearchInfo::setIcon(QString icon)
{
    _icon = icon;
}

// AddFriendApply 实现
AddFriendApply::AddFriendApply(int from_uid, QString name, QString desc, QString icon, QString nick, int sex)
    : _from_uid(from_uid), _name(name), _desc(desc), _icon(icon), _nick(nick), _sex(sex)
{
}

// FriendInfo 实现
void FriendInfo::AppendChatMsgs(const std::vector<std::shared_ptr<TextChatData>> text_vec)
{
    _chat_msgs.insert(_chat_msgs.end(), text_vec.begin(), text_vec.end());
}