#pragma once
#include "SearchList.h"
#include "Singleton.h"
#include "UserData.h"
#include <QByteArray>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <qobject.h>

class TextChatMsg;
struct ImageChatMsg;
struct UserProfile;
struct FriendApplyNotify;
struct AuthAcceptedPeer;

class TcpMgr : public QObject, public Singleton<TcpMgr>
{
    Q_OBJECT
    friend class Singleton<TcpMgr>;

public:
    enum class ReconnectState
    {
        Idle,
        Connecting,
        Backoff,
        LoggedIn
    };

    ~TcpMgr() override;

private:
    TcpMgr();
    void initHandlers();
    void handleMsg(ReqId id, int len, QByteArray data);

    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onConnected();
    void onLoginSuccess(const QJsonObject &json_obj);
    void onLoginFailed(int err);

    void enterState(ReconnectState state);
    void startReconnect();
    void doConnect();
    int nextBackoffMs() const;
    bool isTokenError(int err) const;
    void sendCachedLogin();
    void cacheLogin(const ServerInfo &si);
    void processPendingSendQueue();

    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;

    ReconnectState _reconnect_state{ReconnectState::Idle};
    int _reconnect_attempt{0};
    QTimer _reconnect_timer;
    ServerInfo _cached_server_info;
    bool _is_reconnect_login{false};
    bool _pending_disconnect_is_reconnect{false};
    std::vector<std::pair<ReqId, QString>> _pending_send_queue;

public slots:
    void slot_tcp_connect(ServerInfo si);
    bool slot_send_data(ReqId reqId, QString data);
    void slot_heartbeat_abort();
    void slot_heartbeat_timeout();

public:
    void registerHandler(
        ReqId id,
        std::function<void(ReqId id, int len, QByteArray data)> handler);
    void requestChatHistory(int peer_uid, qint64 before_id = 0, int limit = 100);
    void requestAllFriendsChatHistory();
    void requestFileServer(int uid);
    void notifyFileTransferDone(int uid);
    ReconnectState reconnectState() const;

signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId, QString data);
    void sig_switch_chatdlg();
    void sig_login_failed(int);
    void sig_kicked_by_other(QString reason);
    void sig_user_search(std::shared_ptr<UserProfile> profile);
    void sig_add_friend_failed(int err);
    void sig_friend_apply(std::shared_ptr<FriendApplyNotify> apply_info);
    void sig_auth_rsp(std::shared_ptr<AuthAcceptedPeer> peer);
    void sig_add_auth_friend(std::shared_ptr<AuthAcceptedPeer> peer);
    void sig_text_chat_msg(std::shared_ptr<TextChatMsg> msg_ptr);
    void sig_image_chat_msg(std::shared_ptr<ImageChatMsg> msg_ptr);
    void sig_chat_history(int peer_uid, std::vector<std::shared_ptr<TextChatData>> msgs);
    void sig_file_transfer_rsp(int err, QString host, QString port, QString token);
    void sig_reconnect_state_changed(TcpMgr::ReconnectState state, int next_retry_ms);
    void sig_reconnect_failed_token_expired();
};
