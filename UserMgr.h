#pragma once

#include "Singleton.h"
#include <QObject>

class UserMgr : public QObject , public Singleton<UserMgr>
{
    Q_OBJECT
    friend class Singleton<UserMgr>;
public:
    ~UserMgr() override;
    void setName(const QString& name);
    void setToken(const QString& token);
    void setUid(int uid);
private:
    UserMgr();
    UserMgr(const UserMgr&) = delete;
    UserMgr& operator=(const UserMgr&) = delete;
    QString _name;
    QString _token;
    int _uid;
};
