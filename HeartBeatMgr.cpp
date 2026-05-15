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
        qDebug() << "heartbeat pong received";
        // TODO: 收到 Pong 清零未响应计数
    });
}

void HeartBeatMgr::onTcpLoginOk()
{
    start();
}

void HeartBeatMgr::stop()
{
    _ping_timer.stop();
    _running = false;
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
    TcpMgr &tcp = TcpMgr::getInstance();
    bumpMissedPongIfNeeded();
    disconnectIfMissedTooMany();

    tcp.slot_send_data(ID_HEARTBEAT_PING, HEARTBEAT_EMPTY_BODY);
}

void HeartBeatMgr::bumpMissedPongIfNeeded()
{
    // TODO: 在未收到上一轮 Pong 时递增计数，用于断开判定
}

void HeartBeatMgr::disconnectIfMissedTooMany()
{
    // TODO: 超过 HEARTBEAT_MAX_MISSED_PONG 时断开并上报 UI（见 MainWindow TODO）
}
