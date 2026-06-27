#ifndef CHATPAGE_H
#define CHATPAGE_H

#include "global.h"
#include <QList>
#include <QMap>
#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ChatPage;
}
QT_END_NAMESPACE

class PictureBubble;
struct FriendListEntry;
struct TextChatData;

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage() override;
    void setFriendEntry(std::shared_ptr<FriendListEntry> peer);
    void appendChatMsg(std::shared_ptr<TextChatData> msg);

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

    // 等待异步下载完成的图片气泡：URL -> bubble 列表
    QMap<QString, QList<PictureBubble *>> _image_bubbles;

private slots:
    void on_send_btn_clicked();

signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};

#endif // CHATPAGE_H
