#pragma once

#include "Singleton.h"
#include "UserData.h"
#include "UserModels.h"
#include <QObject>
#include <memory>
#include <unordered_map>

class UserMgr : public QObject, public Singleton<UserMgr>
{
    Q_OBJECT
    friend class Singleton<UserMgr>;

public:
    ~UserMgr() override;
    void setSelfProfile(std::shared_ptr<SelfProfile> profile);
    void setToken(const QString &token);
    int getUid() const;
    QString getName() const;
    std::shared_ptr<SelfProfile> getSelfProfile() const;

    bool alreadyApply(std::shared_ptr<PendingFriendApplyRow> apply_info);
    bool alreadyApply(int from_uid);
    void addApply(std::shared_ptr<PendingFriendApplyRow> apply_info);
    void removeApply(std::shared_ptr<PendingFriendApplyRow> apply_info);
    void removeApply(int from_uid);
    std::shared_ptr<PendingFriendApplyRow> getApply(int from_uid) const;
    const std::unordered_map<int, std::shared_ptr<PendingFriendApplyRow>> &getApplyList() const;
    void appendApplyList(const QJsonArray &apply_list);

    bool checkFriendById(int uid);
    void addFriend(std::shared_ptr<AuthAcceptedPeer> peer);
    std::shared_ptr<FriendListEntry> getFriendById(int uid);
    void removeFriend(int uid);
    void appendFriendList(const QJsonArray &friend_list);

    std::vector<std::shared_ptr<FriendListEntry>> getChatListPerpage();
    bool isLoadChatFinish();
    void updateChatLoadedCount();
    std::vector<std::shared_ptr<FriendListEntry>> getContactListPerpage();
    void updateContactLoadedCount();
    bool isLoadContactFinish();
    void appendFriendChatMsg(int uid, const std::vector<std::shared_ptr<TextChatData>> &msg_vec);
    void clearSession();

private:
    UserMgr();
    UserMgr(const UserMgr &) = delete;
    UserMgr &operator=(const UserMgr &) = delete;

    QString _token;
    std::unordered_map<int, std::shared_ptr<PendingFriendApplyRow>> _apply_list;
    std::shared_ptr<SelfProfile> _self_profile;
    QMap<int, std::shared_ptr<FriendListEntry>> _friend_map;
    std::vector<std::shared_ptr<FriendListEntry>> _friend_list;
    int _chat_loaded;
    int _contact_loaded;
};
