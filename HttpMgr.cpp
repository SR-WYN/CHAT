#include "HttpMgr.h"
#include <qnetworkaccessmanager.h>
#include <qnetworkrequest.h>
#include <qstringview.h>

HttpMgr::~HttpMgr()
{
}

HttpMgr::HttpMgr()
{
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::postHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    QNetworkReply *reply = _manager.post(request, data);
    connect(reply, &QNetworkReply::finished, [this, reply, req_id, mod]() {
        // 处理错误情况
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << reply->errorString();
            // 发送信号通知完成
            emit this->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }
        // 无错误
        QString res = reply->readAll();
        // 发送信号通知完成
        emit this->sig_http_finish(req_id, res, ErrorCodes::SUCCESS, mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if (mod == Modules::REGISTERMOD)
    {
        // 发送信号通知指定模块http的响应结束了
        emit sig_reg_mod_finish(id, res, err);
    }

    if (mod == Modules::RESETMOD)
    {
        // 发送信号通知指定模块http的响应结束了
        emit sig_reset_mod_finish(id, res, err);
    }

    if (mod == Modules::LOGINMOD)
    {
        // 发送信号通知指定模块http的响应结束了
        emit sig_login_mod_finish(id, res, err);
    }
}
