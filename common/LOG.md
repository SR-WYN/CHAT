# CHAT 客户端日志使用说明

## 1. 日志写到哪里

`config.json` 中 `Log.Dir` 为日志目录的**绝对路径**。所有 `.log` 直接放在该目录下，例如：

`/home/asus/NETLEARN/CHAT/src/logdir/app.log`

| 枚举 `LogModule` | 文件名 | 用途 |
|------------------|--------|------|
| `App` | `app.log` | 客户端启动、退出（`main.cpp`） |
| `Config` | `config.log` | 读取 Gate 地址、日志配置（`ConfigMgr`） |
| `Tcp` | `tcp.log` | 与 ChatServer 长连接、收发协议（`TcpMgr`） |
| `Http` | `http.log` | 注册、登录、验证码等 HTTP（`HttpMgr`） |
| `User` | `user.log` | 本地用户资料、好友列表、聊天记录缓存（`UserMgr`） |
| `Ui` | `ui.log` | 各窗口与列表交互（`MainWindow`、`ChatDialog`、`LoginDialog` 等界面 `.cpp`） |

只有**实际打过日志**的模块才会生成对应文件。

---

## 2. 配置 `config.json`

```json
"Log": {
  "Dir": "/home/asus/NETLEARN/CHAT/src/logdir",
  "Level": "info"
}
```

`Level`：`trace` / `debug` / `info` / `warn` / `error` / `critical` / `off`。

---

## 3. 初始化（`main` 里，`QApplication` 与 `ConfigMgr` 之后）

```cpp
#include "ConfigMgr.h"
#include "Log.h"

QApplication a(argc, argv);
ConfigMgr::getInstance();
if (!Log::init("CHAT", ConfigMgr::getInstance().getLogConfig())) {
    return 1;
}

// 退出前
Log::shutdown();
```

---

## 4. 在业务代码里打日志

```cpp
#include "Log.h"

Log::info(LogModule::Tcp, "connected {}:{}", host.toStdString(), port);
Log::error(LogModule::Ui, "ChatDialog: friend row missing uid={}", uid);

LOGI(LogModule::Http, "POST {}", path.toStdString());
```

### 4.1 级别

`Log::trace/debug/info/warn/error/critical`，宏 `LOGT` `LOGD` `LOGI` `LOGW` `LOGE` `LOGC`。

### 4.2 格式

- 使用 `{}` 占位。
- `QString` 等 Qt 类型需 `.toStdString()` 或转成 `int` 等再传入。

### 4.3 新增模块

新增模块：改 [`LogModule.h`](LogModule.h) 内 `LogModule` 枚举、`LogNames::_xxx` 与 `_table`（顺序一致）。

命名约定：成员变量 `_snake_case`；成员函数小写驼峰。

## 文件说明

`LogModule.h`、`Log.h`、`Log.cpp` 三个源文件；配置见 `ConfigMgr::getLogConfig()`。

---

## 5. 按文件类型选哪个模块（速查）

| 代码 | 使用 |
|------|------|
| `main.cpp` | `LogModule::App` |
| `ConfigMgr.*` | `LogModule::Config` |
| `TcpMgr.*` | `LogModule::Tcp` |
| `HttpMgr.*` | `LogModule::Http` |
| `UserMgr.*` | `LogModule::User` |
| 其余界面、对话框、列表 | `LogModule::Ui` |

---

## 6. 注意

- 不要用 `qDebug()` 与新日志混用同一功能点；新代码统一用 `Log` + `LogModule`。
- `Log::init` 成功后再写日志。
- 从 `build/` 运行需保证已复制 `config.json`（CMake POST_BUILD 已配置）。
- 仅写文件；无轮转。
