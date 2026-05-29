#pragma once

#include "Singleton.h"
#include "global.h"
#include <QObject>
#include <QTimer>

class TcpMgr;

// TCP 会话层心跳：登录成功后定时 Ping，连续未收到 Pong 则断开
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
    void notePongReceived();

signals:
    void sig_heartbeat_timeout();

private:
    HeartBeatMgr();
    void start();

    QTimer _ping_timer;
    bool _running{false};
    bool _awaiting_pong{false};
    int _missed_pong_count{0};

    void bumpMissedPongIfNeeded();
    void disconnectIfMissedTooMany();

private slots:
    void slot_ping_timer_timeout();
};
