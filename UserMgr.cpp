#include "UserMgr.h"

UserMgr::UserMgr()
{
}

UserMgr::~UserMgr()
{
}

void UserMgr::setName(const QString &name)
{
    _name = name;
}

void UserMgr::setToken(const QString &token)
{
    _token = token;
}

void UserMgr::setUid(int uid)
{
    _uid = uid;
}

int UserMgr::getUid() const
{
    return _uid;
}

QString UserMgr::getName() const
{
    return _name;
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

const std::unordered_map<int,std::shared_ptr<ApplyInfo>>& UserMgr::getApplyList() const
{
    return _apply_list;
}

void UserMgr::setUserInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    if (_user_info)
    {
        _uid = _user_info->_uid;
        _name = _user_info->_name;
    }
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