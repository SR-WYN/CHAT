#include "UserModels.h"
#include "UserData.h"

static QString readNickOrFallback(const QJsonObject &o, const QString &loginName)
{
    QString n = o.contains(QStringLiteral("nick")) ? o[QStringLiteral("nick")].toString() : QString();
    if (n.isEmpty())
    {
        n = loginName;
    }
    return n;
}

UserProfile UserProfile::fromUserJson(const QJsonObject &o, const QString &bioKey)
{
    UserProfile p;
    p.uid = o[QStringLiteral("uid")].toInt();
    p.loginName = o[QStringLiteral("name")].toString();
    p.nick = readNickOrFallback(o, p.loginName);
    p.icon = o.contains(QStringLiteral("icon")) ? o[QStringLiteral("icon")].toString() : QString();
    p.sex = o[QStringLiteral("sex")].toInt();
    p.bio = o.contains(bioKey) ? o[bioKey].toString() : QString();
    p.email = o.contains(QStringLiteral("email")) ? o[QStringLiteral("email")].toString() : QString();
    return p;
}

QString UserProfile::displayNickOrLogin() const
{
    if (!nick.isEmpty())
    {
        return nick;
    }
    return loginName;
}

QString UserProfile::displayNameForList(const QString &alias) const
{
    if (!alias.isEmpty())
    {
        return alias;
    }
    if (!nick.isEmpty())
    {
        return nick;
    }
    return loginName;
}

std::shared_ptr<FriendApplyNotify> FriendApplyNotify::fromNotifyJson(const QJsonObject &o)
{
    auto n = std::make_shared<FriendApplyNotify>();
    const int fromUid = o.contains(QStringLiteral("applyuid")) ? o[QStringLiteral("applyuid")].toInt()
                                                                 : o[QStringLiteral("apply_uid")].toInt();
    QJsonObject shaped = o;
    shaped[QStringLiteral("uid")] = fromUid;
    if (!shaped.contains(QStringLiteral("name")))
    {
        shaped[QStringLiteral("name")] = o[QStringLiteral("name")];
    }
    n->applicant = UserProfile::fromUserJson(shaped);
    n->applicant.uid = fromUid;
    n->applicantAliasToMe =
        o.contains(QStringLiteral("alias_name")) ? o[QStringLiteral("alias_name")].toString() : QString();
    return n;
}

PendingFriendApplyRow::PendingFriendApplyRow(const QJsonObject &o)
    : profile(UserProfile::fromUserJson(o)), status(o[QStringLiteral("status")].toInt()),
      applicantAliasToMe(o.contains(QStringLiteral("alias_name")) ? o[QStringLiteral("alias_name")].toString()
                                                                    : QString())
{
}

PendingFriendApplyRow::PendingFriendApplyRow(const FriendApplyNotify &n, int rowStatus)
    : profile(n.applicant), status(rowStatus), applicantAliasToMe(n.applicantAliasToMe)
{
}

std::shared_ptr<AuthAcceptedPeer> AuthAcceptedPeer::fromAuthJson(const QJsonObject &o)
{
    QJsonObject shaped = o;
    if (!shaped.contains(QStringLiteral("uid")) || shaped[QStringLiteral("uid")].toInt() == 0)
    {
        if (shaped.contains(QStringLiteral("fromuid")))
        {
            shaped[QStringLiteral("uid")] = shaped[QStringLiteral("fromuid")];
        }
    }
    auto p = std::make_shared<AuthAcceptedPeer>();
    p->profile = UserProfile::fromUserJson(shaped);
    p->myAliasForPeer =
        o.contains(QStringLiteral("alias_name")) ? o[QStringLiteral("alias_name")].toString() : QString();
    return p;
}

FriendListEntry::FriendListEntry(const QJsonObject &o)
    : profile(UserProfile::fromUserJson(o)),
      myAliasForPeer(o.contains(QStringLiteral("alias_name")) ? o[QStringLiteral("alias_name")].toString()
                                                              : QString()),
      peerBio(o.contains(QStringLiteral("desc")) ? o[QStringLiteral("desc")].toString() : QString()),
      lastMessage()
{
}

FriendListEntry::FriendListEntry(UserProfile p)
    : profile(std::move(p)), myAliasForPeer(), peerBio(), lastMessage()
{
}

FriendListEntry::FriendListEntry(const AuthAcceptedPeer &peer)
    : profile(peer.profile), myAliasForPeer(peer.myAliasForPeer), peerBio(), lastMessage()
{
}

void FriendListEntry::appendChatMsgs(const std::vector<std::shared_ptr<TextChatData>> &text_vec)
{
    for (const auto &msg : text_vec)
    {
        bool exists = false;
        for (const auto &existing : chat_msgs)
        {
            if (existing->_msg_id == msg->_msg_id)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            chat_msgs.push_back(msg);
        }
    }
    if (!text_vec.empty())
    {
        const auto &msg = text_vec.back();
        lastMessage = msg->_msg_type == ChatMsgType::Image ? QStringLiteral("[图片]")
                                                           : msg->_msg_content;
    }
}

void FriendListEntry::setChatMsgs(const std::vector<std::shared_ptr<TextChatData>> &text_vec)
{
    chat_msgs = text_vec;
    if (!text_vec.empty())
    {
        const auto &msg = text_vec.back();
        lastMessage = msg->_msg_type == ChatMsgType::Image ? QStringLiteral("[图片]")
                                                           : msg->_msg_content;
    }
}

QString FriendListEntry::listDisplayName() const
{
    return profile.displayNameForList(myAliasForPeer);
}
