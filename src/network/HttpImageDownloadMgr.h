#pragma once

#include "Singleton.h"
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPixmap>
#include <QString>

class HttpImageDownloadMgr : public QObject, public Singleton<HttpImageDownloadMgr>
{
    Q_OBJECT
public:
    ~HttpImageDownloadMgr() override;

    /// 异步下载图片。已下载过的 URL 直接返回缓存。
    void downloadImage(const QString &url);

    /// 预加载缓存中的图片
    QPixmap cachedPixmap(const QString &url) const;

private:
    friend class Singleton<HttpImageDownloadMgr>;
    HttpImageDownloadMgr();

    QNetworkAccessManager _manager;
    QMap<QString, QPixmap> _cache;

private slots:
    void onDownloadFinished();

signals:
    void sig_image_download_finished(const QString &url, const QPixmap &pixmap);
};
