#pragma once

#include "Singleton.h"
#include "UserData.h"
#include <QObject>
#include <unordered_map>

class UserMgr : public QObject , public Singleton<UserMgr>
{
    Q_OBJECT
    friend class Singleton<UserMgr>;
public:
    ~UserMgr() override;
    void setUserInfo(std::shared_ptr<UserInfo> user_info);
    void setName(const QString& name);
    void setToken(const QString& token);
    void setUid(int uid);
    int getUid() const;
    QString getName() const;
    bool alreadyApply(std::shared_ptr<ApplyInfo> apply_info);
    bool alreadyApply(int from_uid);
    void addApply(std::shared_ptr<ApplyInfo> apply_info);
    void removeApply(std::shared_ptr<ApplyInfo> apply_info);
    void removeApply(int from_uid);
    const std::unordered_map<int,std::shared_ptr<ApplyInfo>>& getApplyList() const;
    void appendApplyList(const QJsonArray& apply_list);
private:
    UserMgr();
    UserMgr(const UserMgr&) = delete;
    UserMgr& operator=(const UserMgr&) = delete;
    QString _name;
    QString _token;
    int _uid;
    std::unordered_map<int,std::shared_ptr<ApplyInfo>> _apply_list;
    std::shared_ptr<UserInfo> _user_info;
};
