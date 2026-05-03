#include "UserMgr.h"

UserMgr::UserMgr()
{
}

UserMgr::~UserMgr()
{
}


void UserMgr::setToken(const QString &token)
{
    _token = token;
}


int UserMgr::getUid() const
{
    return _user_info->_uid;
}

QString UserMgr::getName() const
{
    return _user_info->_name;
}

bool UserMgr::alreadyApply(std::shared_ptr<ApplyInfo> apply_info)
{
    return alreadyApply(apply_info->_uid);
}

bool UserMgr::alreadyApply(int from_uid)
{
    if (_apply_list.count(from_uid))
    {
        return true;
    }
    return false;
}

void UserMgr::addApply(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_list[apply_info->_uid] = apply_info;
}

void UserMgr::removeApply(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_list.erase(apply_info->_uid);
}

void UserMgr::removeApply(int from_uid)
{
    _apply_list.erase(from_uid);
}

std::shared_ptr<ApplyInfo> UserMgr::getApply(int from_uid) const
{
    auto it = _apply_list.find(from_uid);
    if (it == _apply_list.end())
    {
        return nullptr;
    }
    return it->second;
}

const std::unordered_map<int,std::shared_ptr<ApplyInfo>>& UserMgr::getApplyList() const
{
    return _apply_list;
}

void UserMgr::setUserInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
}

void UserMgr::appendApplyList(const QJsonArray& apply_list)
{
    for (const QJsonValue &value:apply_list)
    {
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto sex = value["sex"].toInt();
        auto uid = value["uid"].toInt();
        auto status = value["status"].toInt();
        auto apply_info = std::make_shared<ApplyInfo>(uid, name, desc, icon, nick, sex, status);
        _apply_list[uid] = apply_info;
    }
}

bool UserMgr::checkFriendById(int uid)
{
    return _friend_map.contains(uid);
}

void UserMgr::addFriend(std::shared_ptr<AuthRsp> auth_rsp)
{
    _friend_map[auth_rsp->_uid] = std::make_shared<FriendInfo>(auth_rsp);
}
void UserMgr::addFriend(std::shared_ptr<AuthInfo> auth_info)
{
    _friend_map[auth_info->_uid] = std::make_shared<FriendInfo>(auth_info);
}
std::shared_ptr<FriendInfo> UserMgr::getFriendById(int uid)
{
    if (_friend_map.contains(uid))
    {
        return _friend_map[uid];
    }
    qDebug() << "not found friend uid is " << uid;
    return nullptr;
}

void UserMgr::removeFriend(int uid)
{
    if (_friend_map.contains(uid))
    {
        _friend_map.remove(uid);
        return;
    }
    qDebug() << "not found friend uid is " << uid;
}