# 项目编码与 Qt 交互规范 (V1.0)

## 1. 代码命名规范

### 1.1 基础变量与命名
* **类名 (Classes):** 采用 **PascalCase**。
    * *示例：* `TcpMgr`, `ChatWindow`, `LoginDialog`
* **成员变量 (Members):** 采用 **下划线前缀 + snake_case**。
    * *示例：* `_socket`, `_user_id`, `_is_connected`
    * *理由：* 一眼区分局部变量与成员属性，避免构造函数赋值时 `this->` 的冗余。
* **局部变量 (Variables):** 采用 **snake_case**。
    * *示例：* `temp_data`, `packet_len`
* **函数名 (Methods):** 采用 **camelCase**。
    * *示例：* `initHandlers()`, `connectServer()`
* **常量/枚举:** 采用 **SCREAMING_SNAKE_CASE**。
    * *示例：* `MAX_RETRY_COUNT`, `ERR_SUCCESS`

### 1.2 文件命名
* **头文件/源文件:** 必须与类名完全一致（大小写敏感）。
    * *示例：* `TcpMgr.h` / `TcpMgr.cpp`

---

## 2. Qt 信号与槽命名规范

### 2.1 信号命名 (Signals)
* **前缀:** 必须以 `sig_` 开头。
* **命名格式:** `sig_<模块/对象>_<动作/状态>`
* **示例:**
    * `sig_con_success(bool is_success)`
    * `sig_login_failed(int code)`
    * `sig_user_avatar_updated(const QImage& image)`

### 2.2 槽函数命名 (Slots)
* **前缀:** 必须以 `slot_` 开头。
* **命名格式:** `slot_<触发源>_<触发原因>`
* **分类规范:**
    1. **响应信号的槽:** `slot_tcp_recv_data()` (响应 `readyRead`)
    2. **响应 UI 的槽:** `slot_btn_login_clicked()` (响应点击)
    3. **内部逻辑中继槽:** `slot_update_timer_timeout()`
