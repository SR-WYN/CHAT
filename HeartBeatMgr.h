#pragma once

#include "Singleton.h"
#include "global.h"
#include <QObject>
#include <QTimer>

class TcpMgr;

// TCP 会话层心跳：TCP 登录成功后定时 Ping，服务端 Pong（超时断开等后续 TODO）
class HeartBeatMgr : public QObject, public Singleton<HeartBeatMgr>
{
    Q_OBJECT
    friend class Singleton<HeartBeatMgr>;

public:
    ~HeartBeatMgr() override;

    void attachToTcpMgr(TcpMgr *tcp_mgr);
    void registerHandlers(TcpMgr *tcp_mgr);
    void onTcpLoginOk();
    void stop();

private:
    HeartBeatMgr();
    void start();

    QTimer _ping_timer;
    bool _running{false};

    void bumpMissedPongIfNeeded();
    void disconnectIfMissedTooMany();

private slots:
    void slot_ping_timer_timeout();
};
