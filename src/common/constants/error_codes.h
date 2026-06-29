// error_codes.h - 错误码定义
// 客户端专有错误码放在 1-99，服务端通用错误码保持与各服务器一致
#pragma once

enum ErrorCodes
{
    SUCCESS = 0,

    // 客户端专有错误码
    ERR_JSON = 1,    // 客户端 JSON 解析失败
    ERR_NETWORK = 2, // 客户端网络错误

    // 服务端通用错误码
    ERROR_JSON = 1001,        // 服务端 JSON 解析错误
    RPC_FAILED = 1002,        // 服务端 RPC 请求错误
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
