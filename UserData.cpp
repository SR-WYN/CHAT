#include "UserData.h"

SearchInfo::SearchInfo(int uid, QString name, QString nick, QString desc, int sex)
{
    _uid = uid;
    _name = name;
    _nick = nick;
    _desc = desc;
    _sex = sex;
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