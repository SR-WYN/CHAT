#pragma once

#include "Singleton.h"
#include "UserData.h"
#include <QObject>
#include <memory>
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
    std::shared_ptr<ApplyInfo> getApply(int from_uid) const;
    const std::unordered_map<int,std::shared_ptr<ApplyInfo>>& getApplyList() const;
    void appendApplyList(const QJsonArray& apply_list);
    bool checkFriendById(int uid);
    void addFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void addFriend(std::shared_ptr<AuthInfo> auth_info);
    std::shared_ptr<FriendInfo> getFriendById(int uid);
    void removeFriend(int uid);
    void appendFriendList(const QJsonArray& friend_list);
    std::vector<std::shared_ptr<FriendInfo>> getChatListPerpage();
    bool isLoadChatFinish();
    void updateChatLoadedCount();
    std::vector<std::shared_ptr<FriendInfo>> getContactListPerpage();
    void updateContactLoadedCount();
    bool isLoadContactFinish();
private:
    UserMgr();
    UserMgr(const UserMgr&) = delete;
    UserMgr& operator=(const UserMgr&) = delete;
    QString _token;
    std::unordered_map<int,std::shared_ptr<ApplyInfo>> _apply_list;
    std::shared_ptr<UserInfo> _user_info;
    QMap<int,std::shared_ptr<FriendInfo>> _friend_map;
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;
    int _chat_loaded;
    int _contact_loaded;
};
