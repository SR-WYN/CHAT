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