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
    ERR_JSON = 1,    // json解析失败
    ERR_NETWORK = 2, // 网络错误
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
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};