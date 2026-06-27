#include "HttpMgr.h"
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
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

void HttpMgr::uploadImages(const QString& host, const QString& port, int uid, const QString& token,
                           const QVector<QString>& paths)
{
    _upload_host = host;
    _upload_port = port;
    _upload_uid = uid;
    _upload_token = token;
    _pending_image_paths = paths;
    _image_upload_results.clear();
    _image_upload_results.reserve(paths.size());

    if (_pending_image_paths.isEmpty())
    {
        emit sig_upload_images_finished(_image_upload_results, ErrorCodes::SUCCESS);
        return;
    }

    startNextImageUpload();
}

void HttpMgr::startNextImageUpload()
{
    if (_pending_image_paths.isEmpty())
    {
        emit sig_upload_images_finished(_image_upload_results, ErrorCodes::SUCCESS);
        return;
    }

    const QString local_path = _pending_image_paths.takeFirst();
    QFileInfo file_info(local_path);
    if (!file_info.exists())
    {
        _image_upload_results.append(qMakePair(local_path, QString()));
        startNextImageUpload();
        return;
    }

    QFile* file = new QFile(local_path);
    if (!file->open(QIODevice::ReadOnly))
    {
        file->deleteLater();
        _image_upload_results.append(qMakePair(local_path, QString()));
        startNextImageUpload();
        return;
    }

    QHttpMultiPart* multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart file_part;
    file_part.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("application/octet-stream"));
    file_part.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"%1\"")
                                     .arg(file_info.fileName())));
    file_part.setBodyDevice(file);
    file->setParent(multi_part);
    multi_part->append(file_part);

    QUrl url(QStringLiteral("http://%1:%2/upload/image").arg(_upload_host, _upload_port));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1").arg(_upload_token).toUtf8());
    request.setRawHeader("X-Uid", QString::number(_upload_uid).toUtf8());

    QNetworkReply* reply = _manager.post(request, multi_part);
    multi_part->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, &HttpMgr::onImageUploadFinished);
    reply->setProperty("local_path", local_path);
}

void HttpMgr::onImageUploadFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
    {
        return;
    }

    const QString local_path = reply->property("local_path").toString();
    QString url;
    ErrorCodes err = ErrorCodes::SUCCESS;

    if (reply->error() != QNetworkReply::NoError)
    {
        err = ErrorCodes::ERR_NETWORK;
    }
    else
    {
        const QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject())
        {
            err = ErrorCodes::ERR_JSON;
        }
        else
        {
            QJsonObject obj = doc.object();
            if (obj["error"].toInt() != ErrorCodes::SUCCESS)
            {
                err = static_cast<ErrorCodes>(obj["error"].toInt(ErrorCodes::ERR_NETWORK));
            }
            else
            {
                url = obj["url"].toString();
            }
        }
    }

    _image_upload_results.append(qMakePair(local_path, url));
    reply->deleteLater();

    if (err != ErrorCodes::SUCCESS)
    {
        // 任一上传失败即终止本次批量，返回已上传结果
        emit sig_upload_images_finished(_image_upload_results, err);
        return;
    }

    startNextImageUpload();
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
