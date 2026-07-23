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

    // 登录凭据与服务端地址持久化（供断线重连使用）
    void setLastCredentials(int uid, const QString &token);
    void setLastServerInfo(const ServerInfo &si);
    bool hasLastCredentials() const;
    int lastUid() const;
    QString lastToken() const;
    ServerInfo lastServerInfo() const;
    void clearCredentials();

    // 被踢或 token 失效时清理凭据并保留 lastServerInfo

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
    void resetChatLoaded();
    std::vector<std::shared_ptr<FriendListEntry>> getContactListPerpage();
    void updateContactLoadedCount();
    bool isLoadContactFinish();
    void appendFriendChatMsg(int uid, const std::vector<std::shared_ptr<TextChatData>> &msg_vec);
    void updateFriendChatMsgUrl(int friend_id, const QString &msg_id, const QString &url);
    void setFriendChatHistory(int friend_id,
                              const std::vector<std::shared_ptr<TextChatData>> &msg_vec);
    void mergeFriendChatHistory(int friend_id,
                                const std::vector<std::shared_ptr<TextChatData>> &msg_vec);
    std::vector<int> getAllFriendUids() const;
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

    // 供断线重连复用的凭据与服务端地址
    int _last_uid{0};
    QString _last_token;
    ServerInfo _last_server_info;
};
