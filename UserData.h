#pragma once

#include <QString>
class SearchInfo
{
public:
    SearchInfo(int uid, QString name, QString nick, QString desc, int sex);
    int getUid() const;
    QString getName() const;
    QString getNick() const;
    QString getDesc() const;
    int getSex() const;
    void setUid(int uid);
    void setName(QString name);
    void setNick(QString nick);
    void setDesc(QString desc);
    void setSex(int sex);

private:
    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;
};