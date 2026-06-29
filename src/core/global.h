#pragma once

#include "QStyle"
#include <QByteArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QString>
#include <QWidget>
#include <functional>
#include <grpcpp/support/server_interceptor.h>
#include <iostream>
#include <memory>
#include <mutex>

// 用来刷新qss
extern std::function<void(QWidget *)> repolish;

extern std::function<QString(QString)> xorString;

constexpr int TIP_OFFSET = 5;
constexpr int MIN_APPLY_LABEL_ED_LEN = 110;
inline const QString ADD_PREFIX = QStringLiteral("添加标签：");

enum ReqId
{
    ID_GET_VERIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002,        // 注册用户
    ID_RESET_PWD = 1003,       // 重置密码
    ID_LOGIN_USER = 1004,      // 登录用户
    ID_LOGOUT_USER = 1005,     // 登出用户
    ID_CHAT_LOGIN = 2001,      // 聊天登录
    ID_CHAT_LOGIN_RSP = 2002,  // 聊天登录响应
    ID_SEARCH_USER_REQ = 2003, // 搜索用户请求
    ID_SEARCH_USER_RSP = 2004, // 搜索用户响应
    ID_ADD_FRIEND_REQ = 2005, // 添加好友请求
    ID_ADD_FRIEND_RSP = 2006, // 添加好友响应
    ID_NOTIFY_ADDFRIEND_REQ = 2007, // 通知添加好友请求
    ID_AUTH_FRIEND_REQ = 2008, // 认证好友请求
    ID_AUTH_FRIEND_RSP = 2009, // 认证好友响应
    ID_NOTIFY_AUTH_FRIEND_REQ = 2010, // 通知认证好友请求
    ID_TEXT_CHAT_MSG_REQ = 2011, // 文本聊天消息请求
    ID_TEXT_CHAT_MSG_RSP = 2012, // 文本聊天消息响应
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 2013, // 通知文本聊天消息请求
    ID_CHAT_HISTORY_REQ = 2014,         // 聊天历史请求
    ID_CHAT_HISTORY_RSP = 2015,         // 聊天历史响应
    ID_FILE_TRANSFER_REQ = 2016,         // 请求 FileServer 地址
    ID_FILE_TRANSFER_RSP = 2017,         // 返回 FileServer 地址+token
    ID_FILE_TRANSFER_DONE = 2018,        // 通知上传完成，删除 token
    ID_IMAGE_CHAT_MSG_REQ = 2019,        // 图片聊天消息请求
    ID_IMAGE_CHAT_MSG_RSP = 2020,        // 图片聊天消息响应
    ID_NOTIFY_IMAGE_CHAT_MSG_REQ = 2021, // 通知图片聊天消息请求
    ID_KICK_NOTIFY = 2022,               // 被踢通知
    ID_HEARTBEAT_PING = 3001, // 心跳ping
    ID_HEARTBEAT_PONG = 3002, // 心跳pong
};

// 心跳：V1 只用间隔发 Ping；其余供后续未回 Pong 断开、带 ts 负载等 TODO
constexpr int HEARTBEAT_PING_INTERVAL_MS = 30'000;
constexpr int HEARTBEAT_MAX_MISSED_PONG = 3;
constexpr int HEARTBEAT_PONG_DEADLINE_MS = HEARTBEAT_PING_INTERVAL_MS * HEARTBEAT_MAX_MISSED_PONG;
inline constexpr char HEARTBEAT_KEY_TS[] = "ts";
inline const QString HEARTBEAT_EMPTY_BODY = QStringLiteral("{}");

enum Modules
{
    REGISTERMOD = 0,
    RESETMOD = 1,
    LOGINMOD = 2,
};

#include "error_codes.h"

enum TipErr
{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFITM = 4,
    TIP_VERIFY_ERR = 5,
    TIP_USER_ERR = 6,
};

enum ClickLabelState
{
    NORMAL = 0,
    SELECTED = 1,
};

struct ServerInfo
{
    QString host;
    QString port;
    QString token;
    int uid;
};

enum ChatUIMode
{
    SEARCH_MODE,
    CHAT_MODE,
    CONTACT_MODE,
};

enum ListItemType
{
    CHAT_USER_ITEM,    // 聊天用户
    CONTACT_USER_ITEM, // 联系人用户
    SEARCH_USER_ITEM,  // 搜索到的用户
    ADD_TIP_USER_ITEM, // 提示添加用户
    INVALID_ITEM,      // 不可点击
    GROUP_TIP_ITEM,    // 分组提示
    LINE_ITEM,         // 分割线
    APPLY_FRIEND_ITEM, // 申请好友
};

enum class ChatRole
{
    SELF,
    OTHER
};

struct MsgInfo
{
    QString msgFlag;
    QString content;
    QPixmap pixmap;
};

constexpr int CHAT_COUNT_PER_PAGE = 10;