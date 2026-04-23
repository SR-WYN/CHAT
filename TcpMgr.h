#pragma once
#include "Singleton.h"
#include <QObject>
#include <QTcpSocket>
#include <qobject.h>

class TcpMgr : public QObject, public Singleton<TcpMgr>
{
    Q_OBJECT
    friend class Singleton<TcpMgr>;

public:
    ~TcpMgr() override;

private:
    TcpMgr();
    void initHandlers();
    void handleMsg(ReqId id, int len, QByteArray data);
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
public slots:
    void slot_tcp_connect(ServerInfo);
    bool slot_send_data(ReqId reqId, QString data);
signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId, QString data);
    void sig_switch_chatdlg();
    void sig_login_failed(int);
};