#pragma once

#include "Singleton.h"
#include "global.h"
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QVector>

// 单个文件上传结果：本地路径 -> 上传后 URL（失败时 URL 为空）
using ImageUploadResult = QPair<QString, QString>;

class HttpUploadMgr : public QObject, public Singleton<HttpUploadMgr>
{
    Q_OBJECT
public:
    ~HttpUploadMgr() override;

    /// 批量上传本地图片到 FileServer。
    /// @param host      FileServer 主机地址
    /// @param port      FileServer HTTP 端口
    /// @param uid       当前用户 ID（用于 X-Uid 头鉴权）
    /// @param token     文件传输临时 token（Bearer 认证）
    /// @param paths     本地图片文件路径列表
    void uploadImages(const QString& host, const QString& port, int uid, const QString& token,
                      const QVector<QString>& paths);

private:
    friend class Singleton<HttpUploadMgr>;
    HttpUploadMgr();

    void startNextUpload();

    QNetworkAccessManager _manager;
    QString _host;
    QString _port;
    int _uid = 0;
    QString _token;
    QVector<QString> _pending_paths;
    QVector<ImageUploadResult> _results;

private slots:
    void onUploadFinished();

signals:
    /// 批量上传完成信号
    void sig_upload_images_finished(const QVector<ImageUploadResult>& results, ErrorCodes err);
};
