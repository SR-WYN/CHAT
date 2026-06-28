#include "HttpMgr.h"
#include "Log.h"
#include "LogModule.h"
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
    LOGI(LogModule::Http, "postHttpReq url={} req_id={} mod={} body_len={}",
         url.toString().toStdString(), static_cast<int>(req_id), static_cast<int>(mod), data.size());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    QNetworkReply *reply = _manager.post(request, data);
    connect(reply, &QNetworkReply::finished, [this, reply, req_id, mod]() {
        if (reply->error() != QNetworkReply::NoError)
        {
            LOGE(LogModule::Http, "postHttpReq failed req_id={} mod={} error={}",
                 static_cast<int>(req_id), static_cast<int>(mod),
                 reply->errorString().toStdString());
            emit this->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }
        QString res = reply->readAll();
        LOGI(LogModule::Http, "postHttpReq finished req_id={} mod={} resp_len={}",
             static_cast<int>(req_id), static_cast<int>(mod), res.size());
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

    LOGI(LogModule::Http, "uploadImages start uid={} count={} host={}:{}", uid, paths.size(),
         host.toStdString(), port.toStdString());

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
        LOGI(LogModule::Http, "uploadImages all done count={}", _image_upload_results.size());
        emit sig_upload_images_finished(_image_upload_results, ErrorCodes::SUCCESS);
        return;
    }

    const QString local_path = _pending_image_paths.takeFirst();
    QFileInfo file_info(local_path);
    if (!file_info.exists())
    {
        LOGW(LogModule::Http, "uploadImages file not found path={}", local_path.toStdString());
        _image_upload_results.append(qMakePair(local_path, QString()));
        startNextImageUpload();
        return;
    }

    QFile* file = new QFile(local_path);
    if (!file->open(QIODevice::ReadOnly))
    {
        LOGW(LogModule::Http, "uploadImages open failed path={}", local_path.toStdString());
        file->deleteLater();
        _image_upload_results.append(qMakePair(local_path, QString()));
        startNextImageUpload();
        return;
    }

    LOGI(LogModule::Http, "uploadImages uploading path={} size={}", local_path.toStdString(),
         file_info.size());

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
        LOGE(LogModule::Http, "onImageUploadFinished network error path={} error={}",
             local_path.toStdString(), reply->errorString().toStdString());
        err = ErrorCodes::ERR_NETWORK;
    }
    else
    {
        const QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject())
        {
            LOGE(LogModule::Http, "onImageUploadFinished invalid json path={} body={}",
                 local_path.toStdString(), data.toStdString());
            err = ErrorCodes::ERR_JSON;
        }
        else
        {
            QJsonObject obj = doc.object();
            if (obj["error"].toInt() != ErrorCodes::SUCCESS)
            {
                err = static_cast<ErrorCodes>(obj["error"].toInt(ErrorCodes::ERR_NETWORK));
                LOGE(LogModule::Http, "onImageUploadFinished server error path={} error={}",
                     local_path.toStdString(), static_cast<int>(err));
            }
            else
            {
                url = obj["url"].toString();
                LOGI(LogModule::Http, "onImageUploadFinished success path={} url={}",
                     local_path.toStdString(), url.toStdString());
            }
        }
    }

    _image_upload_results.append(qMakePair(local_path, url));
    reply->deleteLater();

    if (err != ErrorCodes::SUCCESS)
    {
        emit sig_upload_images_finished(_image_upload_results, err);
        return;
    }

    startNextImageUpload();
}

void HttpMgr::downloadImage(const QString& url)
{
    if (_image_cache.contains(url))
    {
        LOGD(LogModule::Http, "downloadImage cache hit url={}", url.toStdString());
        emit sig_image_download_finished(url, _image_cache[url]);
        return;
    }

    if (_download_token.isEmpty())
    {
        LOGW(LogModule::Http, "downloadImage no auth,abort url = {}",url.toStdString());
        emit sig_image_download_finished(url, QPixmap());
        return;
    }
    LOGI(LogModule::Http, "downloadImage url={}", url.toStdString());
    QNetworkRequest request{url};
    request.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1").arg(_download_token).toUtf8());
    request.setRawHeader("X-Uid", QString::number(_download_uid).toUtf8());
    QNetworkReply* reply = _manager.get(request);
    connect(reply, &QNetworkReply::finished, this, &HttpMgr::onImageDownloadFinished);
    reply->setProperty("image_url", url);
}

QPixmap HttpMgr::cachedImage(const QString& url) const
{
    auto it = _image_cache.find(url);
    return it != _image_cache.end() ? it.value() : QPixmap();
}

void HttpMgr::onImageDownloadFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
    {
        return;
    }

    const QString url = reply->property("image_url").toString();
    QPixmap pixmap;
    if (reply->error() == QNetworkReply::NoError)
    {
        pixmap.loadFromData(reply->readAll());
        if (!pixmap.isNull())
        {
            _image_cache[url] = pixmap;
            LOGI(LogModule::Http, "onImageDownloadFinished success url={} size={}x{}",
                 url.toStdString(), pixmap.width(), pixmap.height());
        }
        else
        {
            LOGE(LogModule::Http, "onImageDownloadFinished decode failed url={}",
                 url.toStdString());
        }
    }
    else
    {
        LOGE(LogModule::Http, "onImageDownloadFinished failed url={} error={}",
             url.toStdString(), reply->errorString().toStdString());
    }
    reply->deleteLater();
    emit sig_image_download_finished(url, pixmap);
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if (mod == Modules::REGISTERMOD)
    {
        emit sig_reg_mod_finish(id, res, err);
    }

    if (mod == Modules::RESETMOD)
    {
        emit sig_reset_mod_finish(id, res, err);
    }

    if (mod == Modules::LOGINMOD)
    {
        emit sig_login_mod_finish(id, res, err);
    }
}

void HttpMgr::setDownloadAuth(const QString& host,const QString& port,int uid,const QString& token)
{
    _download_host = host;
    _download_port = port;
    _download_uid = uid;
    _download_token = token;
    LOGI(LogModule::Http, "setDownloadAuth host={} port={} uid={} token={}",
         host.toStdString(), port.toStdString(), uid, token.toStdString());
}

void HttpMgr::clearDownloadAuth()
{
    _download_host.clear();
    _download_port.clear();
    _download_uid = 0;
    _download_token.clear();
    LOGI(LogModule::Http, "clearDownloadAuth");
}