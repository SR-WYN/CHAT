#pragma once

#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

/** 搜索到的用户：name=登录名，nick=user 表昵称 */
class SearchInfo
{
public:
    SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon = "");
    int getUid() const;
    QString getName() const;
    QString getNick() const;
    QString getDesc() const;
    int getSex() const;
    QString getIcon() const;
    void setUid(int uid);
    void setName(QString name);
    void setNick(QString nick);
    void setDesc(QString desc);
    void setSex(int sex);
    void setIcon(QString icon);

private:
    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;
    QString _icon;
};

/** 好友申请通知：对方 user.nick 为昵称 */
class AddFriendApply
{
public:
    AddFriendApply(int from_uid, QString name, QString desc, QString icon, QString nick, int sex,
                   QString alias_name = QString());
    int _from_uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int _sex;
    /** 申请人对当前用户（被申请人）的备注，对应 friend_apply.alias_name */
    QString _alias_name;
};

struct ApplyInfo
{
    ApplyInfo(int uid, QString name, QString desc, QString icon, QString nick, int sex, int status,
              QString alias_name = QString())
        : _uid(uid), _name(name), _desc(desc), _icon(icon), _nick(nick), _sex(sex), _status(status),
          _alias_name(alias_name)
    {
    }

    ApplyInfo(std::shared_ptr<AddFriendApply> addinfo)
        : _uid(addinfo->_from_uid), _name(addinfo->_name), _desc(addinfo->_desc),
          _icon(addinfo->_icon), _nick(addinfo->_nick), _sex(addinfo->_sex), _status(0),
          _alias_name(addinfo->_alias_name)
    {
    }
    void SetIcon(QString head) { _icon = head; }
    int _uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int _sex;
    int _status;
    QString _alias_name;
};

/** 认证通过通知里的对方：nick=对方昵称，alias=我对对方的备注（friend.alias_name） */
struct AuthInfo
{
    AuthInfo(int uid, QString name, QString nick, QString icon, int sex, QString alias = QString())
        : _uid(uid), _name(name), _nick(nick), _icon(icon), _sex(sex), _alias(alias)
    {
    }
    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    QString _alias;
};

struct AuthRsp
{
    AuthRsp(int peer_uid, QString peer_name, QString peer_nick, QString peer_icon, int peer_sex,
            QString peer_alias = QString())
        : _uid(peer_uid), _name(peer_name), _nick(peer_nick), _icon(peer_icon), _sex(peer_sex),
          _alias(peer_alias)
    {
    }

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    QString _alias;
};

struct TextChatData;
/** 好友：nick=对方 user 昵称，alias=friend 表对 TA 的备注 */
struct FriendInfo
{
    FriendInfo(int uid, QString name, QString nick, QString icon, int sex, QString desc,
               QString alias, QString last_msg = "")
        : _uid(uid), _name(name), _nick(nick), _icon(icon), _sex(sex), _desc(desc), _alias(alias),
          _last_msg(last_msg)
    {
    }

    FriendInfo(std::shared_ptr<AuthInfo> auth_info)
        : _uid(auth_info->_uid), _nick(auth_info->_nick), _icon(auth_info->_icon),
          _name(auth_info->_name), _sex(auth_info->_sex), _desc(""),
          _alias(auth_info->_alias)
    {
    }

    FriendInfo(std::shared_ptr<AuthRsp> auth_rsp)
        : _uid(auth_rsp->_uid), _nick(auth_rsp->_nick), _icon(auth_rsp->_icon),
          _name(auth_rsp->_name), _sex(auth_rsp->_sex), _desc(""),
          _alias(auth_rsp->_alias)
    {
    }

    void AppendChatMsgs(const std::vector<std::shared_ptr<TextChatData>> text_vec);

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    QString _desc;
    QString _alias;
    QString _last_msg;
    std::vector<std::shared_ptr<TextChatData>> _chat_msgs;
};

struct UserInfo
{
    /** 当前登录用户等：nick 为本人昵称（user.nick） */
    UserInfo(int uid, QString name, QString nick, QString icon, int sex, QString last_msg = "")
        : _uid(uid), _name(name), _nick(nick), _icon(icon), _sex(sex), _last_msg(last_msg), _alias()
    {
    }

    UserInfo(std::shared_ptr<AuthInfo> auth)
        : _uid(auth->_uid), _name(auth->_name), _nick(auth->_nick), _icon(auth->_icon),
          _sex(auth->_sex), _last_msg(""), _alias(auth->_alias)
    {
    }

    UserInfo(int uid, QString name, QString icon)
        : _uid(uid), _name(name), _icon(icon), _nick(name), _sex(0), _last_msg(""), _alias()
    {
    }

    UserInfo(std::shared_ptr<AuthRsp> auth)
        : _uid(auth->_uid), _name(auth->_name), _nick(auth->_nick), _icon(auth->_icon),
          _sex(auth->_sex), _last_msg(""), _alias(auth->_alias)
    {
    }

    UserInfo(std::shared_ptr<SearchInfo> search_info)
        : _uid(search_info->getUid()), _name(search_info->getName()), _nick(search_info->getNick()),
          _icon(search_info->getIcon()), _sex(search_info->getSex()), _last_msg(""), _alias()
    {
    }

    UserInfo(std::shared_ptr<FriendInfo> friend_info)
        : _uid(friend_info->_uid), _name(friend_info->_name), _nick(friend_info->_nick),
          _icon(friend_info->_icon), _sex(friend_info->_sex), _last_msg(""),
          _alias(friend_info->_alias)
    {
        _chat_msgs = friend_info->_chat_msgs;
    }

    /** 列表展示：好友备注 > 对方昵称 > 登录名 */
    QString displayName() const
    {
        if (!_alias.isEmpty())
        {
            return _alias;
        }
        if (!_nick.isEmpty())
        {
            return _nick;
        }
        return _name;
    }

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    QString _last_msg;
    QString _alias;
    std::vector<std::shared_ptr<TextChatData>> _chat_msgs;
};

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
