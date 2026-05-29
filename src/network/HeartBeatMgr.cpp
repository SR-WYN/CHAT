#include "HeartBeatMgr.h"
#include "TcpMgr.h"
#include <qdebug.h>

HeartBeatMgr::HeartBeatMgr()
{
    _ping_timer.setInterval(HEARTBEAT_PING_INTERVAL_MS);
    _ping_timer.setSingleShot(false);
    QObject::connect(&_ping_timer, &QTimer::timeout, this, &HeartBeatMgr::slot_ping_timer_timeout);
}

HeartBeatMgr::~HeartBeatMgr()
{
    stop();
}

void HeartBeatMgr::attachToTcpMgr(TcpMgr *tcp_mgr)
{
    Q_UNUSED(tcp_mgr);
}

void HeartBeatMgr::registerHandlers(TcpMgr *tcp_mgr)
{
    if (!tcp_mgr)
    {
        return;
    }

    tcp_mgr->registerHandler(ReqId::ID_HEARTBEAT_PONG, [](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        Q_UNUSED(data);
        HeartBeatMgr::getInstance().notePongReceived();
    });
}

void HeartBeatMgr::notePongReceived()
{
    _missed_pong_count = 0;
    _awaiting_pong = false;
}

void HeartBeatMgr::onTcpLoginOk()
{
    _missed_pong_count = 0;
    _awaiting_pong = false;
    start();
}

void HeartBeatMgr::stop()
{
    _ping_timer.stop();
    _running = false;
    _awaiting_pong = false;
    _missed_pong_count = 0;
}

void HeartBeatMgr::start()
{
    if (_running)
    {
        return;
    }
    _running = true;
    _ping_timer.start();
}

void HeartBeatMgr::slot_ping_timer_timeout()
{
    bumpMissedPongIfNeeded();
    disconnectIfMissedTooMany();
    if (!_running)
    {
        return;
    }

    TcpMgr &tcp = TcpMgr::getInstance();
    tcp.slot_send_data(ID_HEARTBEAT_PING, HEARTBEAT_EMPTY_BODY);
    _awaiting_pong = true;
}

void HeartBeatMgr::bumpMissedPongIfNeeded()
{
    if (_awaiting_pong)
    {
        ++_missed_pong_count;
    }
}

void HeartBeatMgr::disconnectIfMissedTooMany()
{
    if (_missed_pong_count < HEARTBEAT_MAX_MISSED_PONG)
    {
        return;
    }
    stop();
    emit sig_heartbeat_timeout();
}
