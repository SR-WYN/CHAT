#include "Log.h"
#include "TcpMgr.h"
#include "HeartBeatMgr.h"
#include "UserData.h"
#include "UserModels.h"
#include "UserMgr.h"
#include "global.h"
#include <QJsonDocument>
#include <cstdint>
#include <memory>
#include <utility>
#include <qabstractsocket.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qimage.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qobject.h>
#include <qtcpsocket.h>
#include <QTimer>

namespace
{
QString reqIdName(ReqId id)
{
    switch (id)
    {
    case ID_CHAT_LOGIN_RSP:
        return QStringLiteral("ID_CHAT_LOGIN_RSP");
    case ID_SEARCH_USER_RSP:
        return QStringLiteral("ID_SEARCH_USER_RSP");
    case ID_ADD_FRIEND_RSP:
        return QStringLiteral("ID_ADD_FRIEND_RSP");
    case ID_NOTIFY_ADDFRIEND_REQ:
        return QStringLiteral("ID_NOTIFY_ADDFRIEND_REQ");
    case ID_AUTH_FRIEND_RSP:
        return QStringLiteral("ID_AUTH_FRIEND_RSP");
    case ID_NOTIFY_AUTH_FRIEND_REQ:
        return QStringLiteral("ID_NOTIFY_AUTH_FRIEND_REQ");
    case ID_TEXT_CHAT_MSG_RSP:
        return QStringLiteral("ID_TEXT_CHAT_MSG_RSP");
    case ID_NOTIFY_TEXT_CHAT_MSG_REQ:
        return QStringLiteral("ID_NOTIFY_TEXT_CHAT_MSG_REQ");
    case ID_CHAT_HISTORY_RSP:
        return QStringLiteral("ID_CHAT_HISTORY_RSP");
    case ID_FILE_TRANSFER_RSP:
        return QStringLiteral("ID_FILE_TRANSFER_RSP");
    case ID_IMAGE_CHAT_MSG_RSP:
        return QStringLiteral("ID_IMAGE_CHAT_MSG_RSP");
    case ID_NOTIFY_IMAGE_CHAT_MSG_REQ:
        return QStringLiteral("ID_NOTIFY_IMAGE_CHAT_MSG_REQ");
    case ID_HEARTBEAT_PONG:
        return QStringLiteral("ID_HEARTBEAT_PONG");
    default:
        return QStringLiteral("UNKNOWN");
    }
}
} // namespace

TcpMgr::TcpMgr() : _host(""), _port(0), _b_recv_pending(false), _message_id(0), _message_len(0)
{
    QObject::connect(&_socket, &QTcpSocket::connected, [&]() {
        LOGI(LogModule::Tcp, "connected to {}:{}", _host.toStdString(), _port);
        emit sig_con_success(true);
    });

    QObject::connect(&_socket, &QTcpSocket::readyRead, [&]() {
        _buffer.append(_socket.readAll());

        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_0);

        while (true)
        {
            if (!_b_recv_pending)
            {
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2))
                {
                    return;
                }
                stream >> _message_id >> _message_len;
                _buffer = _buffer.mid(sizeof(quint16) * 2);
            }

            if (_buffer.size() < _message_len)
            {
                _b_recv_pending = true;
                return;
            }
            _b_recv_pending = false;
            QByteArray messageBody = _buffer.mid(0, _message_len);
            _buffer = _buffer.mid(_message_len);
            LOGD(LogModule::Tcp, "recv msg_id={} name={} len={}", _message_id,
                 reqIdName(static_cast<ReqId>(_message_id)).toStdString(), _message_len);
            handleMsg(ReqId(_message_id), _message_len, messageBody);
        }
    });

    QObject::connect(&_socket, &QTcpSocket::errorOccurred, this,
                     [this](QAbstractSocket::SocketError error) {
                         LOGE(LogModule::Tcp, "socket error host={}:{} error={}",
                              _host.toStdString(), _port, static_cast<int>(error));
                     });

    QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
        LOGI(LogModule::Tcp, "disconnected from {}:{}", _host.toStdString(), _port);
        HeartBeatMgr::getInstance().stop();
    });
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
    initHandlers();
    HeartBeatMgr::getInstance().attachToTcpMgr(this);
    HeartBeatMgr::getInstance().registerHandlers(this);
    QObject::connect(&HeartBeatMgr::getInstance(), &HeartBeatMgr::sig_heartbeat_timeout, this,
                     &TcpMgr::slot_heartbeat_abort);
}

TcpMgr::~TcpMgr()
{
}

void TcpMgr::initHandlers()
{
    _handlers.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        QJsonDocument jsondoc = QJsonDocument::fromJson(data);

        if (jsondoc.isNull())
        {
            LOGE(LogModule::Tcp, "ID_CHAT_LOGIN_RSP invalid json");
            emit sig_login_failed(ErrorCodes::ERR_JSON);
            return;
        }

        QJsonObject json_obj = jsondoc.object();

        if (!json_obj.contains(QStringLiteral("error")))
        {
            LOGE(LogModule::Tcp, "ID_CHAT_LOGIN_RSP missing error field");
            emit sig_login_failed(ErrorCodes::ERR_JSON);
            return;
        }

        int err = json_obj[QStringLiteral("error")].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGE(LogModule::Tcp, "ID_CHAT_LOGIN_RSP error={}", err);
            emit sig_login_failed(err);
            return;
        }
        auto self = std::make_shared<SelfProfile>(UserProfile::fromUserJson(json_obj));
        UserMgr::getInstance().setSelfProfile(self);
        LOGI(LogModule::Tcp, "ID_CHAT_LOGIN_RSP success uid={}", self->uid);
        if (json_obj.contains(QStringLiteral("token")))
        {
            const QString tok = json_obj[QStringLiteral("token")].toString();
            if (!tok.isEmpty())
            {
                UserMgr::getInstance().setToken(tok);
                LOGI(LogModule::Tcp, "ID_CHAT_LOGIN_RSP token set len={}", tok.length());
            }
        }

        if (json_obj.contains(QStringLiteral("apply_list")))
        {
            UserMgr::getInstance().appendApplyList(json_obj[QStringLiteral("apply_list")].toArray());
        }

        if (json_obj.contains(QStringLiteral("friend_list")))
        {
            UserMgr::getInstance().appendFriendList(json_obj[QStringLiteral("friend_list")].toArray());
        }

        emit sig_switch_chatdlg();
        HeartBeatMgr::getInstance().onTcpLoginOk();
        QTimer::singleShot(300, this, [this]() { requestAllFriendsChatHistory(); });
    });

    _handlers.insert(ID_SEARCH_USER_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);

        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_SEARCH_USER_RSP invalid json");
            emit sig_user_search(nullptr);
            return;
        }

        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_SEARCH_USER_RSP missing error field");
            emit sig_user_search(nullptr);
            return;
        }

        auto err = json_obj[QStringLiteral("error")].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_SEARCH_USER_RSP error={}", err);
            emit sig_user_search(nullptr);
            return;
        }

        auto profile = std::make_shared<UserProfile>(UserProfile::fromUserJson(json_obj));
        LOGI(LogModule::Tcp, "ID_SEARCH_USER_RSP found uid={} name={}", profile->uid,
             profile->loginName.toStdString());
        emit sig_user_search(profile);
    });

    _handlers.insert(ID_ADD_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);

        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_ADD_FRIEND_RSP invalid json");
            emit sig_add_friend_failed(ErrorCodes::ERR_JSON);
            return;
        }

        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_ADD_FRIEND_RSP missing error field");
            emit sig_add_friend_failed(ErrorCodes::ERR_JSON);
            return;
        }

        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_ADD_FRIEND_RSP error={}", err);
            emit sig_add_friend_failed(err);
            return;
        }
        LOGI(LogModule::Tcp, "ID_ADD_FRIEND_RSP success");
    });

    _handlers.insert(ID_NOTIFY_ADDFRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);

        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_ADDFRIEND_REQ invalid json");
            return;
        }

        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_ADDFRIEND_REQ missing error field");
            return;
        }

        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_NOTIFY_ADDFRIEND_REQ error={}", err);
            return;
        }

        auto notify = FriendApplyNotify::fromNotifyJson(json_obj);
        LOGI(LogModule::Tcp, "ID_NOTIFY_ADDFRIEND_REQ from uid={}", notify->applicant.uid);
        emit sig_friend_apply(notify);
    });

    _handlers.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_AUTH_FRIEND_RSP invalid json");
            return;
        }
        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_AUTH_FRIEND_RSP missing error field");
            return;
        }
        auto err = json_obj[QStringLiteral("error")].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_AUTH_FRIEND_RSP error={}", err);
            return;
        }
        auto peer = AuthAcceptedPeer::fromAuthJson(json_obj);
        LOGI(LogModule::Tcp, "ID_AUTH_FRIEND_RSP accepted uid={}", peer->profile.uid);
        emit sig_auth_rsp(peer);
    });

    _handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_AUTH_FRIEND_REQ invalid json");
            return;
        }
        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_AUTH_FRIEND_REQ missing error field");
            return;
        }
        auto err = json_obj[QStringLiteral("error")].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_NOTIFY_AUTH_FRIEND_REQ error={}", err);
            return;
        }
        auto peer = AuthAcceptedPeer::fromAuthJson(json_obj);
        LOGI(LogModule::Tcp, "ID_NOTIFY_AUTH_FRIEND_REQ from uid={}", peer->profile.uid);
        emit sig_add_auth_friend(peer);
    });

    _handlers.insert(ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull())
        {
            LOGE(LogModule::Tcp, "ID_TEXT_CHAT_MSG_RSP invalid json");
            return;
        }

        QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_TEXT_CHAT_MSG_RSP missing error field");
            return;
        }

        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_TEXT_CHAT_MSG_RSP error={}", err);
            return;
        }
        LOGI(LogModule::Tcp, "ID_TEXT_CHAT_MSG_RSP success");
    });

    _handlers.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull())
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_TEXT_CHAT_MSG_REQ invalid json");
            return;
        }
        QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_TEXT_CHAT_MSG_REQ missing error field");
            return;
        }
        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_NOTIFY_TEXT_CHAT_MSG_REQ error={}", err);
            return;
        }
        auto msg_ptr =
            std::make_shared<TextChatMsg>(json_obj["fromuid"].toInt(), json_obj["touid"].toInt(),
                                          json_obj["text_array"].toArray());
        LOGI(LogModule::Tcp, "ID_NOTIFY_TEXT_CHAT_MSG_REQ from={} to={} count={}",
             msg_ptr->_from_uid, msg_ptr->_to_uid, static_cast<int>(msg_ptr->_chat_msgs.size()));
        UserMgr::getInstance().appendFriendChatMsg(msg_ptr->_from_uid, msg_ptr->_chat_msgs);
        emit sig_text_chat_msg(msg_ptr);
    });

    _handlers.insert(ID_CHAT_HISTORY_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_CHAT_HISTORY_RSP invalid json");
            return;
        }
        const QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains(QStringLiteral("error")))
        {
            LOGE(LogModule::Tcp, "ID_CHAT_HISTORY_RSP missing error field");
            return;
        }
        if (json_obj[QStringLiteral("error")].toInt() != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_CHAT_HISTORY_RSP error={}",
                 json_obj[QStringLiteral("error")].toInt());
            return;
        }
        if (!json_obj.contains(QStringLiteral("peer_uid")) ||
            !json_obj.contains(QStringLiteral("text_array")))
        {
            LOGE(LogModule::Tcp, "ID_CHAT_HISTORY_RSP missing peer_uid/text_array");
            return;
        }
        const int peer_uid = json_obj[QStringLiteral("peer_uid")].toInt();
        const QJsonArray text_array = json_obj[QStringLiteral("text_array")].toArray();
        std::vector<std::shared_ptr<TextChatData>> msgs;
        msgs.reserve(static_cast<size_t>(text_array.size()));
        for (const auto &item : text_array)
        {
            const QJsonObject obj = item.toObject();
            const QString msgid = obj[QStringLiteral("msgid")].toString();
            const QString content = obj[QStringLiteral("content")].toString();
            const int fromuid = obj[QStringLiteral("fromuid")].toInt();
            const int touid = obj[QStringLiteral("touid")].toInt();
            const int msg_type = obj[QStringLiteral("msg_type")].toInt();
            const QString url = obj[QStringLiteral("url")].toString();
            msgs.push_back(std::make_shared<TextChatData>(
                msgid, content, fromuid, touid,
                msg_type == 1 ? ChatMsgType::Image : ChatMsgType::Text, url));
        }
        LOGI(LogModule::Tcp, "ID_CHAT_HISTORY_RSP peer={} count={}", peer_uid,
             static_cast<int>(msgs.size()));
        UserMgr::getInstance().mergeFriendChatHistory(peer_uid, msgs);
        emit sig_chat_history(peer_uid, msgs);
    });

    _handlers.insert(ID_FILE_TRANSFER_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_FILE_TRANSFER_RSP invalid json");
            emit sig_file_transfer_rsp(ErrorCodes::ERR_JSON, QString(), QString(), QString());
            return;
        }
        QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_FILE_TRANSFER_RSP missing error field");
            emit sig_file_transfer_rsp(ErrorCodes::ERR_JSON, QString(), QString(), QString());
            return;
        }
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_FILE_TRANSFER_RSP error={}", err);
            emit sig_file_transfer_rsp(err, QString(), QString(), QString());
            return;
        }
        LOGI(LogModule::Tcp, "ID_FILE_TRANSFER_RSP host={}:{}",
             json_obj["host"].toString().toStdString(), json_obj["port"].toString().toStdString());
        emit sig_file_transfer_rsp(err, json_obj["host"].toString(), json_obj["port"].toString(),
                                   json_obj["token"].toString());
    });

    _handlers.insert(ID_IMAGE_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_IMAGE_CHAT_MSG_RSP invalid json");
            return;
        }
        QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains("error") || json_obj["error"].toInt() != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_IMAGE_CHAT_MSG_RSP error={}", json_obj["error"].toInt());
            return;
        }
        LOGI(LogModule::Tcp, "ID_IMAGE_CHAT_MSG_RSP success");
    });

    _handlers.insert(ID_NOTIFY_IMAGE_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_IMAGE_CHAT_MSG_REQ invalid json");
            return;
        }
        QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            LOGE(LogModule::Tcp, "ID_NOTIFY_IMAGE_CHAT_MSG_REQ missing error field");
            return;
        }
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            LOGW(LogModule::Tcp, "ID_NOTIFY_IMAGE_CHAT_MSG_REQ error={}", err);
            return;
        }
        auto msg_ptr = std::make_shared<ImageChatMsg>(json_obj["fromuid"].toInt(),
                                                      json_obj["touid"].toInt(),
                                                      json_obj["image_array"].toArray());
        LOGI(LogModule::Tcp, "ID_NOTIFY_IMAGE_CHAT_MSG_REQ from={} to={} count={}",
             msg_ptr->_from_uid, msg_ptr->_to_uid, static_cast<int>(msg_ptr->_chat_msgs.size()));
        for (const auto& img : msg_ptr->_chat_msgs)
        {
            UserMgr::getInstance().appendFriendChatMsg(
                img->_from_uid,
                {std::make_shared<TextChatData>(img->_msg_id, QString(), img->_from_uid,
                                                 img->_to_uid, ChatMsgType::Image,
                                                 img->_url)});
        }
        emit sig_image_chat_msg(msg_ptr);
    });
}

void TcpMgr::requestChatHistory(int peer_uid, qint64 before_id, int limit)
{
    QJsonObject obj;
    obj[QStringLiteral("uid")] = UserMgr::getInstance().getUid();
    obj[QStringLiteral("peer_uid")] = peer_uid;
    obj[QStringLiteral("before_id")] = before_id;
    obj[QStringLiteral("limit")] = limit;
    const QByteArray json_data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    LOGI(LogModule::Tcp, "requestChatHistory peer={} before_id={} limit={}", peer_uid, before_id,
         limit);
    slot_send_data(ID_CHAT_HISTORY_REQ, QString::fromUtf8(json_data));
}

void TcpMgr::requestAllFriendsChatHistory()
{
    const auto uids = UserMgr::getInstance().getAllFriendUids();
    LOGI(LogModule::Tcp, "requestAllFriendsChatHistory count={}", static_cast<int>(uids.size()));
    for (const int peer_uid : uids)
    {
        requestChatHistory(peer_uid, 0, 100);
    }
}

void TcpMgr::requestFileServer(int uid)
{
    QJsonObject obj;
    obj[QStringLiteral("uid")] = uid;
    const QByteArray json_data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    LOGI(LogModule::Tcp, "requestFileServer uid={}", uid);
    slot_send_data(ID_FILE_TRANSFER_REQ, QString::fromUtf8(json_data));
}

void TcpMgr::notifyFileTransferDone(int uid)
{
    QJsonObject obj;
    obj[QStringLiteral("uid")] = uid;
    const QByteArray json_data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    LOGI(LogModule::Tcp, "notifyFileTransferDone uid={}", uid);
    slot_send_data(ID_FILE_TRANSFER_DONE, QString::fromUtf8(json_data));
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if (find_iter == _handlers.end())
    {
        LOGW(LogModule::Tcp, "handleMsg no handler msg_id={} name={}", static_cast<int>(id),
             reqIdName(id).toStdString());
        return;
    }

    find_iter.value()(id, len, data);
}

void TcpMgr::registerHandler(ReqId id, std::function<void(ReqId id, int len, QByteArray data)> handler)
{
    LOGD(LogModule::Tcp, "registerHandler msg_id={} name={}", static_cast<int>(id),
         reqIdName(id).toStdString());
    _handlers.insert(id, std::move(handler));
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    HeartBeatMgr::getInstance().stop();
    _host = si.host;
    _port = static_cast<uint16_t>(si.port.toInt());
    LOGI(LogModule::Tcp, "connecting to {}:{}", _host.toStdString(), _port);
    _socket.connectToHost(_host, _port);
}

void TcpMgr::slot_heartbeat_abort()
{
    LOGI(LogModule::Tcp, "heartbeat abort");
    if (_socket.state() != QAbstractSocket::UnconnectedState)
    {
        _socket.abort();
    }
}

bool TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t msg_id = reqId;
    QByteArray data_Bytes = data.toUtf8();
    quint16 len = static_cast<quint16>(data_Bytes.size());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    out << msg_id << len;
    block.append(data_Bytes);
    LOGD(LogModule::Tcp, "send msg_id={} name={} len={}", msg_id,
         reqIdName(static_cast<ReqId>(reqId)).toStdString(), len);
    _socket.write(block);
    return true;
}
