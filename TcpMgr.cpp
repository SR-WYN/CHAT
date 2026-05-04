#include "TcpMgr.h"
#include "UserData.h"
#include "UserMgr.h"
#include "global.h"
#include <QJsonDocument>
#include <cstdint>
#include <memory>
#include <qabstractsocket.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qimage.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qobject.h>
#include <qtcpsocket.h>

TcpMgr::TcpMgr() : _host(""), _port(0), _b_recv_pending(false), _message_id(0), _message_len(0)
{
    QObject::connect(&_socket, &QTcpSocket::connected, [&]() {
        qDebug() << "Connected to serve!";
        // 连接建立后发送消息
        emit sig_con_success(true);
    });

    QObject::connect(&_socket, &QTcpSocket::readyRead, [&]() {
        // 当有数据可读时，读取所有数据
        // 读取所有数据并追加到缓冲区
        _buffer.append(_socket.readAll());

        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_0);

        while (true)
        {
            // 先解析头部
            if (!_b_recv_pending)
            {
                // 检查缓冲区的数据是否足够解析出消息头
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2))
                {
                    return;
                }
                stream >> _message_id >> _message_len;

                // 将buffer中前四个字节移除
                _buffer = _buffer.mid(sizeof(quint16) * 2);

                // 输出读取的数据
                qDebug() << "Message ID: " << _message_id;
                qDebug() << "Message Length: " << _message_len;
            }

            // buffer剩余长度是否满足消息体长度，不满足则继续读取
            if (_buffer.size() < _message_len)
            {
                _b_recv_pending = true;
                return;
            }
            _b_recv_pending = false;
            // 解析消息体
            QByteArray messageBody = _buffer.mid(0, _message_len);
            qDebug() << "recvive body msg is " << messageBody;
            _buffer = _buffer.mid(_message_len);
            handleMsg(ReqId(_message_id), _message_len, messageBody);
        }
    });

    QObject::connect(&_socket, &QTcpSocket::errorOccurred, this,
                     [this](QAbstractSocket::SocketError error) {
                         qDebug() << "Error:" << _socket.errorString();
                     });

    // 处理连接断开
    QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server ";
    });
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
    // 注册消息
    initHandlers();
}

TcpMgr::~TcpMgr()
{
}

void TcpMgr::initHandlers()
{
    // 登录响应
    _handlers.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);

        qDebug() << "handle id is " << id;
        qDebug() << "data is " << data;
        // 将QByteArray转换成QDocument
        QJsonDocument jsondoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsondoc.isNull())
        {
            qDebug() << "QJsonDocument is null";
            return;
        }

        QJsonObject json_obj = jsondoc.object();

        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is " << err;
            emit sig_login_failed(err);
            return;
        }

        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Login Failed, err is " << err;
            emit sig_login_failed(err);
            return;
        }
        auto uid = json_obj["uid"].toInt();
        auto name = json_obj["name"].toString();
        QString nick =
            json_obj.contains(QStringLiteral("nick")) ? json_obj["nick"].toString() : QString();
        if (nick.isEmpty())
        {
            nick = name;
        }
        auto icon = json_obj["icon"].toString();
        auto sex = json_obj["sex"].toInt();

        auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex);
        UserMgr::getInstance().setUserInfo(user_info);
        UserMgr::getInstance().setToken(json_obj["token"].toString());

        if (json_obj.contains("apply_list"))
        {
            UserMgr::getInstance().appendApplyList(json_obj["apply_list"].toArray());
        }

        // 获取好友列表
        if (json_obj.contains("friend_list"))
        {
            UserMgr::getInstance().appendFriendList(json_obj["friend_list"].toArray());
        }

        emit sig_switch_chatdlg();
    });

    // 搜索用户响应
    _handlers.insert(ID_SEARCH_USER_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);

        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            qDebug() << "search rsp json parse failed";
            emit sig_user_search(nullptr);
            return;
        }

        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            qDebug() << "search rsp missing error field";
            emit sig_user_search(nullptr);
            return;
        }

        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "search failed, err is " << err;
            emit sig_user_search(nullptr);
            return;
        }

        const QString icon =
            json_obj.contains(QStringLiteral("icon")) ? json_obj["icon"].toString() : QString();
        QString snick =
            json_obj.contains(QStringLiteral("nick")) ? json_obj["nick"].toString() : QString();
        const QString sname = json_obj["name"].toString();
        if (snick.isEmpty())
        {
            snick = sname;
        }
        auto si = std::make_shared<SearchInfo>(json_obj["uid"].toInt(), sname, snick,
                                               json_obj["desc"].toString(), json_obj["sex"].toInt(),
                                               icon);
        emit sig_user_search(si);
    });

    // 添加好友响应
    _handlers.insert(ID_ADD_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);

        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            qDebug() << "add friend rsp json parse failed";
            emit sig_add_friend_failed(ErrorCodes::ERR_JSON);
            return;
        }

        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            qDebug() << "add friend rsp missing error field";
            emit sig_add_friend_failed(ErrorCodes::ERR_JSON);
            return;
        }

        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "add friend failed, err is " << err;
            emit sig_add_friend_failed(ErrorCodes::ERR_JSON);
            return;
        }
        qDebug() << "get add friend rsp success";
    });

    // 通知添加好友响应
    _handlers.insert(ID_NOTIFY_ADDFRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);

        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            qDebug() << "notify add friend rsp json parse failed";
            return;
        }

        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            qDebug() << "notify add friend rsp json parse failed";
            return;
        }

        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "notify add friend failed, err is " << err;
            return;
        }

        int from_uid = json_obj.contains(QStringLiteral("applyuid"))
                           ? json_obj[QStringLiteral("applyuid")].toInt()
                           : json_obj[QStringLiteral("apply_uid")].toInt();
        QString from_name = json_obj["name"].toString();
        QString from_desc = json_obj["desc"].toString();
        QString from_icon = json_obj["icon"].toString();
        int from_sex = json_obj["sex"].toInt();
        QString from_nick =
            json_obj.contains(QStringLiteral("nick")) ? json_obj["nick"].toString() : QString();
        if (from_nick.isEmpty())
        {
            from_nick = from_name;
        }
        const QString from_alias = json_obj.contains(QStringLiteral("alias_name"))
                                       ? json_obj["alias_name"].toString()
                                       : QString();
        auto apply_info = std::make_shared<AddFriendApply>(
            from_uid, from_name, from_desc, from_icon, from_nick, from_sex, from_alias);
        emit sig_friend_apply(apply_info);

        qDebug() << "notify add friend success";
    });

    // 认证好友响应
    _handlers.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            qDebug() << "auth friend rsp json parse failed";
            return;
        }
        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            qDebug() << "auth friend rsp missing error field";
            return;
        }
        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "auth friend failed, err is " << err;
            return;
        }
        auto name = json_obj["name"].toString();
        QString nick =
            json_obj.contains(QStringLiteral("nick")) ? json_obj["nick"].toString() : QString();
        if (nick.isEmpty())
        {
            nick = name;
        }
        const QString alias = json_obj.contains(QStringLiteral("alias_name"))
                                  ? json_obj["alias_name"].toString()
                                  : QString();
        auto icon = json_obj["icon"].toString();
        auto sex = json_obj["sex"].toInt();
        auto uid = json_obj["uid"].toInt();
        auto rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex, alias);
        qDebug() << "auth friend success";
        emit sig_auth_rsp(rsp);
    });

    // 通知认证好友响应
    _handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        auto json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull() || !json_doc.isObject())
        {
            qDebug() << "notify auth friend rsp json parse failed";
            return;
        }
        auto json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            qDebug() << "notify auth friend rsp missing error field";
            return;
        }
        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "notify auth friend failed, err is " << err;
            return;
        }

        int from_uid = json_obj["fromuid"].toInt();
        QString name = json_obj["name"].toString();
        QString nick =
            json_obj.contains(QStringLiteral("nick")) ? json_obj["nick"].toString() : QString();
        if (nick.isEmpty())
        {
            nick = name;
        }
        const QString alias = json_obj.contains(QStringLiteral("alias_name"))
                                  ? json_obj["alias_name"].toString()
                                  : QString();
        QString icon = json_obj["icon"].toString();
        int sex = json_obj["sex"].toInt();
        auto auth_info = std::make_shared<AuthInfo>(from_uid, name, nick, icon, sex, alias);

        qDebug() << "notify auth friend success";
        emit sig_add_auth_friend(auth_info);
    });

    // 通知文本聊天
    _handlers.insert(ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull())
        {
            qDebug() << "notify text chat rsp json parse failed";
            return;
        }

        QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "notify text chat rsp json parse failed, err is " << err;
            return;
        }

        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "notify text chat failed, err is " << err;
            return;
        }
        qDebug() << "notify text chat success";
    });

    // 通知文本聊天响应
    _handlers.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(id);
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        if (json_doc.isNull())
        {
            qDebug() << "notify text chat rsp json parse failed";
        }
        QJsonObject json_obj = json_doc.object();
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "notify text chat rsp json parse failed, err is " << err;
            return;
        }
        auto err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "notify text chat failed, err is " << err;
            return;
        }
        qDebug() << "Receive Text Chat Notify Success";
        auto msg_ptr =
            std::make_shared<TextChatMsg>(json_obj["fromuid"].toInt(), json_obj["touid"].toInt(),
                                          json_obj["text_array"].toArray());
        emit sig_text_chat_msg(msg_ptr);
    });
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if (find_iter == _handlers.end())
    {
        qDebug() << "not found id [" << id << "] to handle";
        return;
    }

    find_iter.value()(id, len, data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug() << "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";
    _host = si.host;
    _port = static_cast<uint16_t>(si.port.toInt());
    _socket.connectToHost(_host, _port);
}

bool TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t msg_id = reqId;
    QByteArray data_Bytes = data.toUtf8();
    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(data_Bytes.size());
    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入消息ID和长度
    // id和长度使用网络字节序，所以不直接写入，而是使用QDataStream
    out << msg_id << len;
    // 添加字符串数据
    block.append(data_Bytes);
    // 发送数据
    _socket.write(block);
    return true;
}