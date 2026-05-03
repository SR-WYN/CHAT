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
};

enum Modules
{
    REGISTERMOD = 0,
    RESETMOD = 1,
    LOGINMOD = 2,
};

enum ErrorCodes
{
    SUCCESS = 0,
    ERR_JSON = 1,             // 客户端json解析失败
    ERR_NETWORK = 2,          // 客户端网络错误
    ERROR_JSON = 1001,        // 服务端JSON解析错误
    RPCFAILED = 1002,         // 服务端RPC请求错误
    VERIFY_EXPIRED = 1003,    // 验证码过期
    VERIFY_CODE_ERROR = 1004, // 验证码错误
    USER_EXIST = 1005,        // 用户已存在
    PASSWD_ERROR = 1006,      // 密码错误
    EMAIL_NOT_MATCH = 1007,   // 邮箱不匹配
    PASSWD_UP_FAILED = 1008,  // 密码更新失败
    PASSWD_INVALID = 1009,    // 密码无效
    PASSWD_NOT_MATCH = 1010,  // 密码不匹配
    UID_INVALID = 1011,       // 用户不存在
    TOKEN_INVALID = 1012,     // 令牌无效
};

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