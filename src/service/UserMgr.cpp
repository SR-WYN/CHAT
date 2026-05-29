#include "UserMgr.h"
#include <QDebug>
#include <QJsonObject>
#include <memory>
#include <vector>

UserMgr::UserMgr() : _self_profile(nullptr), _chat_loaded(0), _contact_loaded(0)
{
}

UserMgr::~UserMgr()
{
}

void UserMgr::setToken(const QString &token)
{
    _token = token;
}

int UserMgr::getUid() const
{
    return _self_profile ? _self_profile->uid : 0;
}

QString UserMgr::getName() const
{
    return _self_profile ? _self_profile->loginName : QString();
}

bool UserMgr::alreadyApply(std::shared_ptr<PendingFriendApplyRow> apply_info)
{
    return alreadyApply(apply_info->profile.uid);
}

bool UserMgr::alreadyApply(int from_uid)
{
    return _apply_list.count(from_uid) != 0;
}

void UserMgr::addApply(std::shared_ptr<PendingFriendApplyRow> apply_info)
{
    _apply_list[apply_info->profile.uid] = apply_info;
}

void UserMgr::removeApply(std::shared_ptr<PendingFriendApplyRow> apply_info)
{
    _apply_list.erase(apply_info->profile.uid);
}

void UserMgr::removeApply(int from_uid)
{
    _apply_list.erase(from_uid);
}

std::shared_ptr<PendingFriendApplyRow> UserMgr::getApply(int from_uid) const
{
    auto it = _apply_list.find(from_uid);
    if (it == _apply_list.end())
    {
        return nullptr;
    }
    return it->second;
}

const std::unordered_map<int, std::shared_ptr<PendingFriendApplyRow>> &UserMgr::getApplyList() const
{
    return _apply_list;
}

void UserMgr::setSelfProfile(std::shared_ptr<SelfProfile> profile)
{
    _self_profile = std::move(profile);
}

void UserMgr::appendApplyList(const QJsonArray &apply_list)
{
    for (const QJsonValue &value : apply_list)
    {
        const QJsonObject jo = value.toObject();
        auto row = std::make_shared<PendingFriendApplyRow>(jo);
        _apply_list[row->profile.uid] = row;
    }
}

bool UserMgr::checkFriendById(int uid)
{
    return _friend_map.contains(uid);
}

void UserMgr::addFriend(std::shared_ptr<AuthAcceptedPeer> peer)
{
    auto entry = std::make_shared<FriendListEntry>(*peer);
    _friend_map[peer->profile.uid] = entry;
}

std::shared_ptr<FriendListEntry> UserMgr::getFriendById(int uid)
{
    if (_friend_map.contains(uid))
    {
        return _friend_map[uid];
    }
    return nullptr;
}

void UserMgr::removeFriend(int uid)
{
    if (_friend_map.contains(uid))
    {
        _friend_map.remove(uid);
        return;
    }
}

void UserMgr::appendFriendList(const QJsonArray &friend_list)
{
    for (const QJsonValue &value : friend_list)
    {
        const QJsonObject jo = value.toObject();
        auto info = std::make_shared<FriendListEntry>(jo);
        _friend_list.push_back(info);
        _friend_map[info->uid()] = info;
    }
}

std::vector<std::shared_ptr<FriendListEntry>> UserMgr::getChatListPerpage()
{
    std::vector<std::shared_ptr<FriendListEntry>> friend_list;
    int begin = _chat_loaded;
    int end = begin + CHAT_COUNT_PER_PAGE;
    if (begin >= _friend_list.size())
    {
        return friend_list;
    }

    if (end > _friend_list.size())
    {
        friend_list =
            std::vector<std::shared_ptr<FriendListEntry>>(_friend_list.begin() + begin, _friend_list.end());
        return friend_list;
    }

    friend_list = std::vector<std::shared_ptr<FriendListEntry>>(_friend_list.begin() + begin,
                                                                 _friend_list.begin() + end);
    return friend_list;
}

bool UserMgr::isLoadChatFinish()
{
    return _chat_loaded >= _friend_list.size();
}

void UserMgr::updateChatLoadedCount()
{
    int begin = _chat_loaded;
    int end = begin + CHAT_COUNT_PER_PAGE;
    if (begin >= _friend_list.size())
    {
        return;
    }

    if (end > _friend_list.size())
    {
        _chat_loaded = static_cast<int>(_friend_list.size());
        return;
    }

    _chat_loaded = end;
}

std::vector<std::shared_ptr<FriendListEntry>> UserMgr::getContactListPerpage()
{
    std::vector<std::shared_ptr<FriendListEntry>> friend_list;
    int begin = _contact_loaded;
    int end = begin + CHAT_COUNT_PER_PAGE;
    if (begin >= _friend_list.size())
    {
        return friend_list;
    }

    if (end > _friend_list.size())
    {
        friend_list =
            std::vector<std::shared_ptr<FriendListEntry>>(_friend_list.begin() + begin, _friend_list.end());
        return friend_list;
    }

    friend_list = std::vector<std::shared_ptr<FriendListEntry>>(_friend_list.begin() + begin,
                                                                 _friend_list.begin() + end);
    return friend_list;
}

void UserMgr::updateContactLoadedCount()
{
    int begin = _contact_loaded;
    int end = begin + CHAT_COUNT_PER_PAGE;
    if (begin >= _friend_list.size())
    {
        return;
    }

    if (end > _friend_list.size())
    {
        _contact_loaded = static_cast<int>(_friend_list.size());
        return;
    }

    _contact_loaded = end;
}

bool UserMgr::isLoadContactFinish()
{
    return _contact_loaded >= _friend_list.size();
}

std::shared_ptr<SelfProfile> UserMgr::getSelfProfile() const
{
    return _self_profile;
}

void UserMgr::appendFriendChatMsg(int friend_id, const std::vector<std::shared_ptr<TextChatData>> &msg_vec)
{
    auto find_iter = _friend_map.find(friend_id);
    if (find_iter == _friend_map.end())
    {
        return;
    }
    find_iter.value()->appendChatMsgs(msg_vec);
}

void UserMgr::setFriendChatHistory(int friend_id,
                                   const std::vector<std::shared_ptr<TextChatData>> &msg_vec)
{
    auto find_iter = _friend_map.find(friend_id);
    if (find_iter == _friend_map.end())
    {
        return;
    }
    find_iter.value()->setChatMsgs(msg_vec);
}

void UserMgr::mergeFriendChatHistory(int friend_id,
                                     const std::vector<std::shared_ptr<TextChatData>> &msg_vec)
{
    if (msg_vec.empty())
    {
        return;
    }
    auto find_iter = _friend_map.find(friend_id);
    if (find_iter == _friend_map.end())
    {
        return;
    }
    find_iter.value()->appendChatMsgs(msg_vec);
}

std::vector<int> UserMgr::getAllFriendUids() const
{
    std::vector<int> uids;
    uids.reserve(_friend_list.size());
    for (const auto &entry : _friend_list)
    {
        uids.push_back(entry->uid());
    }
    return uids;
}

void UserMgr::clearSession()
{
    _token.clear();
    _apply_list.clear();
    _self_profile.reset();
    _friend_map.clear();
    _friend_list.clear();
    _chat_loaded = 0;
    _contact_loaded = 0;
}
