#pragma once

#include "QStyle"
#include <QByteArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QWidget>
#include <functional>
#include <grpcpp/support/server_interceptor.h>
#include <iostream>
#include <memory>
#include <mutex>

// 用来刷新qss
extern std::function<void(QWidget *)> repolish;

extern std::function<QString(QString)> xorString;

enum ReqId
{
    ID_GET_VERIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002,        // 注册用户
    ID_RESET_PWD = 1003,       // 重置密码
    ID_LOGIN_USER = 1004,      // 登录用户
    ID_CHAT_LOGIN = 2001,      // 聊天登录
    ID_CHAT_LOGIN_RSP = 2002,  // 登录响应
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
    ERR_JSON = 1,    // 客户端json解析失败
    ERR_NETWORK = 2, // 客户端网络错误
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