#pragma once

#include <memory>
#include <vector>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

struct TextChatData;

/** 与服务端 JSON 对齐的公开身份（不含密码、token） */
struct UserProfile
{
    int uid = 0;
    QString loginName;
    QString nick;
    QString icon;
    int sex = 0;
    /** 对应 JSON desc */
    QString bio;
    /** 邮箱 */
    QString email;

    static UserProfile fromUserJson(const QJsonObject &o,
                                    const QString &bioKey = QStringLiteral("desc"));

    QString displayNickOrLogin() const;
    /** 列表：备注优先 > 昵称 > 登录名 */
    QString displayNameForList(const QString &alias) const;
};

/** 当前登录用户资料（与 UserProfile 同形；token 仍由 UserMgr 单独保存） */
using SelfProfile = UserProfile;

/** TCP MSG_NOTIFY_ADDFRIEND：申请人快照 */
struct FriendApplyNotify
{
    UserProfile applicant;
    /** friend_apply.alias_name：对方申请时对我的备注 */
    QString applicantAliasToMe;

    static std::shared_ptr<FriendApplyNotify> fromNotifyJson(const QJsonObject &o);
};

/** 好友申请列表行（登录 apply_list） */
struct PendingFriendApplyRow
{
    UserProfile profile;
    int status = 0;
    QString applicantAliasToMe;

    explicit PendingFriendApplyRow(const QJsonObject &o);
    explicit PendingFriendApplyRow(const FriendApplyNotify &n, int rowStatus = 0);
};

/** 合并原 AuthInfo / AuthRsp：认证通过后的对方资料 + 我对其备注 */
struct AuthAcceptedPeer
{
    UserProfile profile;
    QString myAliasForPeer;

    static std::shared_ptr<AuthAcceptedPeer> fromAuthJson(const QJsonObject &o);
};

/** 好友列表与会话侧栏：对方资料 + 好友备注 + 可选简介 + 聊天缓存 */
struct FriendListEntry
{
    UserProfile profile;
    QString myAliasForPeer;
    QString peerBio;
    QString lastMessage;
    std::vector<std::shared_ptr<TextChatData>> chat_msgs;

    explicit FriendListEntry(const QJsonObject &o);
    explicit FriendListEntry(const AuthAcceptedPeer &peer);
    explicit FriendListEntry(UserProfile p);
    void appendChatMsgs(const std::vector<std::shared_ptr<TextChatData>> &text_vec);
    void setChatMsgs(const std::vector<std::shared_ptr<TextChatData>> &text_vec);
    QString listDisplayName() const;
    int uid() const { return profile.uid; }
};
