#ifndef CHATPAGE_H
#define CHATPAGE_H

#include "global.h"
#include <QList>
#include <QMap>
#include <QWidget>
#include <memory>
#include <QQueue>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ChatPage;
}
QT_END_NAMESPACE

class PictureBubble;
struct FriendListEntry;
struct TextChatData;

/// FileServer Token 请求用途（用于区分 upload/download 的响应）
enum class FileTransferMode
{
    Upload,
    Download
};

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage() override;
    void setFriendEntry(std::shared_ptr<FriendListEntry> peer);
    void appendChatMsg(std::shared_ptr<TextChatData> msg);

    // 对已入队的待下载 URL 发起下载（先请求 Token，再下载）
    void startImageDownload();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void sendImageBatch(const QVector<QString> &local_paths);
    PictureBubble *appendImageBubble(const QString &image_source, ChatRole role,
                                     bool is_local = true);
    void onFileLabelClicked();
    void onFileTransferRsp(int err, QString host, QString port, QString token);
    void onImageUploadFinished(const QVector<QPair<QString, QString>> &results, ErrorCodes err);
    void onImageDownloadFinished(const QString &url, const QPixmap &pixmap);

    Ui::ChatPage *ui;
    std::shared_ptr<FriendListEntry> _peer;

    // 当前待上传的图片本地路径列表
    QVector<QString> _pending_image_paths;
    // 上传完成后是否已发送图片消息
    bool _uploading_images = false;
    // 当前上传批次的目标好友 uid（切换好友后避免发到错误对象）
    int _upload_target_uid = 0;

    // 等待异步下载完成的图片气泡：URL -> bubble 列表
    QMap<QString, QList<PictureBubble *>> _image_bubbles;

    // 等待异步下载的图片 URL 队列
    QQueue<QString> _pending_download_urls;
    // 是否正在批量下载图片（防止并发请求文件 Token）
    bool _downloading_images = false;

    // 待上传图片本地路径 -> msgid，上传完成后用于更新聊天记录中的 URL
    QMap<QString, QString> _pending_image_msg_ids;

    // 待处理的 FileServer Token 响应用途队列（与请求顺序一一对应）
    QQueue<FileTransferMode> _pending_file_transfer_modes;

private slots:
    void on_send_btn_clicked();

signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};

#endif // CHATPAGE_H
