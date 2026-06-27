#include "HttpImageDownloadMgr.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

HttpImageDownloadMgr::~HttpImageDownloadMgr()
{
}

HttpImageDownloadMgr::HttpImageDownloadMgr()
    : QObject(nullptr)
{
}

void HttpImageDownloadMgr::downloadImage(const QString &url)
{
    if (_cache.contains(url))
    {
        emit sig_image_download_finished(url, _cache[url]);
        return;
    }

    QNetworkRequest request{url};
    QNetworkReply *reply = _manager.get(request);
    connect(reply, &QNetworkReply::finished, this, &HttpImageDownloadMgr::onDownloadFinished);
    reply->setProperty("image_url", url);
}

QPixmap HttpImageDownloadMgr::cachedPixmap(const QString &url) const
{
    auto it = _cache.find(url);
    return it != _cache.end() ? it.value() : QPixmap();
}

void HttpImageDownloadMgr::onDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
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
            _cache[url] = pixmap;
        }
    }
    reply->deleteLater();
    emit sig_image_download_finished(url, pixmap);
}
