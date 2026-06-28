#pragma once

#include "Singleton.h"
#include "global.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QUrl>
#include <QVector>
#include <qtmetamacros.h>

// 单个文件上传结果：本地路径 -> 上传后 URL（失败时 URL 为空）
using ImageUploadResult = QPair<QString, QString>;

class HttpMgr : public QObject, public Singleton<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr() override;
    /// 设置下载图片的认证信息
    /// @param host      FileServer 主机地址
    /// @param port      FileServer HTTP 端口
    /// @param uid       当前用户 ID（用于 X-Uid 头鉴权）
    /// @param token     文件传输临时 token（Bearer 认证）
    void setDownloadAuth(const QString& host,const QString& port,int uid,const QString& token);
    
    /// 清除下载图片的认证信息
    void clearDownloadAuth();

    /// 原有 JSON POST 请求
    void postHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);

    /// 批量上传本地图片到 FileServer。
    /// @param host      FileServer 主机地址
    /// @param port      FileServer HTTP 端口
    /// @param uid       当前用户 ID（用于 X-Uid 头鉴权）
    /// @param token     文件传输临时 token（Bearer 认证）
    /// @param paths     本地图片文件路径列表
    void uploadImages(const QString& host, const QString& port, int uid, const QString& token,
                      const QVector<QString>& paths);

    /// 异步下载图片。已下载过的 URL 直接返回缓存。
    void downloadImage(const QString& url);

    /// 预加载缓存中的图片
    QPixmap cachedImage(const QString& url) const;

private:
    friend class Singleton<HttpMgr>;
    HttpMgr();

    void startNextImageUpload();

    QNetworkAccessManager _manager;

    // 图片上传相关状态
    QString _upload_host;
    QString _upload_port;
    int _upload_uid = 0;
    QString _upload_token;
    QVector<QString> _pending_image_paths;
    QVector<ImageUploadResult> _image_upload_results;

    // 图片下载缓存
    QMap<QString, QPixmap> _image_cache;

    // 图片下载相关状态
    QString _download_host;
    QString _download_port;
    int _download_uid = 0;
    QString _download_token;

private slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void onImageUploadFinished();
    void onImageDownloadFinished();

signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);

    /// 批量图片上传完成信号
    void sig_upload_images_finished(const QVector<ImageUploadResult>& results, ErrorCodes err);

    /// 图片下载完成信号
    void sig_image_download_finished(const QString& url, const QPixmap& pixmap);
};