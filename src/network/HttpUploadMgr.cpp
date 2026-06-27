#include "HttpUploadMgr.h"
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

HttpUploadMgr::~HttpUploadMgr()
{
}

HttpUploadMgr::HttpUploadMgr()
    : QObject(nullptr)
{
}

void HttpUploadMgr::uploadImages(const QString& host, const QString& port, int uid,
                                 const QString& token, const QVector<QString>& paths)
{
    _host = host;
    _port = port;
    _uid = uid;
    _token = token;
    _pending_paths = paths;
    _results.clear();
    _results.reserve(paths.size());

    if (_pending_paths.isEmpty())
    {
        emit sig_upload_images_finished(_results, ErrorCodes::SUCCESS);
        return;
    }

    startNextUpload();
}

void HttpUploadMgr::startNextUpload()
{
    if (_pending_paths.isEmpty())
    {
        emit sig_upload_images_finished(_results, ErrorCodes::SUCCESS);
        return;
    }

    const QString local_path = _pending_paths.takeFirst();
    QFileInfo file_info(local_path);
    if (!file_info.exists())
    {
        _results.append(qMakePair(local_path, QString()));
        startNextUpload();
        return;
    }

    QFile* file = new QFile(local_path);
    if (!file->open(QIODevice::ReadOnly))
    {
        file->deleteLater();
        _results.append(qMakePair(local_path, QString()));
        startNextUpload();
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

    QUrl url(QStringLiteral("http://%1:%2/upload/image").arg(_host, _port));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1").arg(_token).toUtf8());
    request.setRawHeader("X-Uid", QString::number(_uid).toUtf8());

    QNetworkReply* reply = _manager.post(request, multi_part);
    multi_part->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, &HttpUploadMgr::onUploadFinished);
    reply->setProperty("local_path", local_path);
}

void HttpUploadMgr::onUploadFinished()
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

    _results.append(qMakePair(local_path, url));
    reply->deleteLater();

    if (err != ErrorCodes::SUCCESS)
    {
        // 任一上传失败即终止本次批量，返回已上传结果
        emit sig_upload_images_finished(_results, err);
        return;
    }

    startNextUpload();
}
