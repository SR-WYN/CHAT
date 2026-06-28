#include "ChatPage.h"
#include "AnimatedStateWidget.h"
#include "ChatItemBase.h"
#include "HttpMgr.h"
#include "Log.h"
#include "LogModule.h"
#include "MessageTextEdit.h"
#include "PictureBubble.h"
#include "TcpMgr.h"
#include "TextBubble.h"
#include "UserData.h"
#include "UserMgr.h"
#include "UserModels.h"
#include "global.h"
#include "ui_ChatPage.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QStyleOption>
#include <QUuid>

namespace
{
constexpr int kMaxImageWidth = 300;
constexpr int kMaxImageHeight = 400;

QPixmap loadImageScaled(const QString &path)
{
    QPixmap pix(path);
    if (pix.isNull())
    {
        return pix;
    }
    if (pix.width() > kMaxImageWidth || pix.height() > kMaxImageHeight)
    {
        pix = pix.scaled(QSize(kMaxImageWidth, kMaxImageHeight), Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
    }
    return pix;
}
} // namespace

ChatPage::ChatPage(QWidget *parent) : QWidget(parent), ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    ui->emoji_label->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->file_label->setQssInteraction(AnimatedStateWidget::QssInteraction::Momentary);
    ui->emoji_label->setState("normal", "hover", "press", "normal", "hover", "press");
    ui->file_label->setState("normal", "hover", "press", "normal", "hover", "press");

    connect(ui->file_label, &AnimatedStateWidget::clicked, this,
            &ChatPage::onFileLabelClicked);
    connect(&TcpMgr::getInstance(), &TcpMgr::sig_file_transfer_rsp, this,
            &ChatPage::onFileTransferRsp);
    connect(&HttpMgr::getInstance(), &HttpMgr::sig_upload_images_finished, this,
            &ChatPage::onImageUploadFinished);
    connect(&HttpMgr::getInstance(), &HttpMgr::sig_image_download_finished, this,
            &ChatPage::onImageDownloadFinished);
}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::onFileLabelClicked()
{
    const QStringList filters = {QStringLiteral("Image Files (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"),
                                 QStringLiteral("All Files (*)")};
    QStringList file_names = QFileDialog::getOpenFileNames(
        this, QStringLiteral("选择图片"), QString(), filters.join(QStringLiteral(";;")));
    LOGI(LogModule::Ui, "onFileLabelClicked selected count={}", file_names.size());
    for (const QString &file_name : file_names)
    {
        if (!file_name.isEmpty())
        {
            ui->chat_edit->insertImages(file_name);
        }
    }
}

PictureBubble *ChatPage::appendImageBubble(const QString &image_source, ChatRole role,
                                           bool is_local)
{
    auto self_info = UserMgr::getInstance().getSelfProfile();
    if (!self_info)
    {
        return nullptr;
    }

    QString userName = self_info->loginName;
    QString userIcon = self_info->icon;
    if (role == ChatRole::OTHER && _peer)
    {
        userName = _peer->listDisplayName();
        userIcon = _peer->profile.icon;
    }

    ChatItemBase *pChatItem = new ChatItemBase(role);
    pChatItem->setUserName(userName);
    pChatItem->setUserIcon(QPixmap(userIcon));

    QPixmap pix;
    if (is_local)
    {
        pix = loadImageScaled(image_source);
        if (pix.isNull())
        {
            LOGE(LogModule::Ui, "appendImageBubble load failed path={}", image_source.toStdString());
            delete pChatItem;
            return nullptr;
        }
    }
    else
    {
        pix = QPixmap(kMaxImageWidth, kMaxImageHeight);
        pix.fill(Qt::lightGray);
    }

    PictureBubble *pBubble = new PictureBubble(pix, role);
    pChatItem->setWidget(pBubble);
    ui->chat_data_list->appendChatItem(pChatItem);

    if (!is_local)
    {
        _image_bubbles[image_source].append(pBubble);
        // 下载由 appendChatMsg 通过 startImageDownload 统一发起
    }

    return pBubble;
}

void ChatPage::startImageDownload()
{
    if (_pending_download_urls.isEmpty())
    {
        return;
    }
    LOGI(LogModule::Ui, "startImageDownload count={}", _pending_download_urls.size());
    TcpMgr::getInstance().requestFileServer(UserMgr::getInstance().getUid());
}

void ChatPage::on_send_btn_clicked()
{
    if (_peer == nullptr)
    {
        LOGW(LogModule::Ui, "on_send_btn_clicked no peer selected");
        return;
    }
    auto self_info = UserMgr::getInstance().getSelfProfile();
    if (self_info == nullptr)
    {
        return;
    }
    auto peer_info = _peer;
    auto pTextEdit = ui->chat_edit;

    const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
    QJsonObject text_obj;
    QJsonArray text_array;
    int text_size = 0;
    QVector<QString> image_paths;

    LOGI(LogModule::Ui, "on_send_btn_clicked peer={} msg_count={}", peer_info->uid(),
         msgList.size());

    for (int i = 0; i < msgList.size(); ++i)
    {
        const QString type = msgList[i].msgFlag;
        if (type == QStringLiteral("text"))
        {
            if (msgList[i].content.length() > 1024)
            {
                continue;
            }
            QUuid uuid = QUuid::createUuid();
            QString uuid_string = uuid.toString();
            if (text_size + msgList[i].content.length() > 1024)
            {
                text_obj["fromuid"] = self_info->uid;
                text_obj["touid"] = peer_info->uid();
                text_obj["text_array"] = text_array;
                QJsonDocument doc(text_obj);
                QByteArray json_data = doc.toJson(QJsonDocument::Compact);
                text_size = 0;
                text_array = QJsonArray();
                text_obj = QJsonObject();
                emit TcpMgr::getInstance().sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, json_data);
            }
            text_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgid"] = uuid_string;
            text_array.append(obj);
            auto text_msg = std::make_shared<TextChatData>(uuid_string, obj["content"].toString(),
                                                           self_info->uid, peer_info->uid());
            emit sig_append_send_chat_msg(text_msg);
            continue;
        }

        if (type == QStringLiteral("image"))
        {
            image_paths.append(msgList[i].content);
            appendImageBubble(msgList[i].content, ChatRole::SELF);
            continue;
        }

        if (type == QStringLiteral("file"))
        {
            LOGW(LogModule::Ui, "on_send_btn_clicked file type not supported yet");
            continue;
        }
    }

    if (!text_array.isEmpty())
    {
        text_obj["text_array"] = text_array;
        text_obj["fromuid"] = self_info->uid;
        text_obj["touid"] = peer_info->uid();
        QJsonDocument doc(text_obj);
        QByteArray json_data = doc.toJson(QJsonDocument::Compact);
        LOGI(LogModule::Ui, "sending text chat msg peer={} count={}", peer_info->uid(),
             text_array.size());
        emit TcpMgr::getInstance().sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, json_data);
    }

    if (!image_paths.isEmpty())
    {
        sendImageBatch(image_paths);
    }
}

void ChatPage::sendImageBatch(const QVector<QString> &local_paths)
{
    if (_uploading_images)
    {
        _pending_image_paths.append(local_paths);
        LOGI(LogModule::Ui, "sendImageBatch appended to pending count={}", local_paths.size());
        return;
    }
    _pending_image_paths = local_paths;
    _uploading_images = true;
    LOGI(LogModule::Ui, "sendImageBatch start count={}", local_paths.size());
    TcpMgr::getInstance().requestFileServer(UserMgr::getInstance().getUid());
}

void ChatPage::onFileTransferRsp(int err, QString host, QString port, QString token)
{
    if (err != ErrorCodes::SUCCESS)
    {
        LOGE(LogModule::Ui, "onFileTransferRsp error={}", err);
        _uploading_images = false;
        _pending_image_paths.clear();
        _pending_download_urls.clear();
        return;
    }

    // 下载场景优先
    if (!_pending_download_urls.isEmpty())
    {
        LOGI(LogModule::Ui, "onFileTransferRsp download mode host={}:{} count={}",
             host.toStdString(), port.toStdString(), _pending_download_urls.size());
        HttpMgr::getInstance().setDownloadAuth(host, port, UserMgr::getInstance().getUid(), token);
        while (!_pending_download_urls.isEmpty())
        {
            HttpMgr::getInstance().downloadImage(_pending_download_urls.dequeue());
        }
        TcpMgr::getInstance().notifyFileTransferDone(UserMgr::getInstance().getUid());
        return;
    }

    // 上传场景
    if (_pending_image_paths.isEmpty())
    {
        _uploading_images = false;
        return;
    }
    LOGI(LogModule::Ui, "onFileTransferRsp upload mode host={}:{} token_len={}",
         host.toStdString(), port.toStdString(), token.length());
    HttpMgr::getInstance().uploadImages(host, port, UserMgr::getInstance().getUid(), token,
                                        _pending_image_paths);
}

void ChatPage::onImageUploadFinished(const QVector<QPair<QString, QString>> &results,
                                     ErrorCodes err)
{
    auto self_info = UserMgr::getInstance().getSelfProfile();
    if (!self_info || !_peer)
    {
        LOGE(LogModule::Ui, "onImageUploadFinished no self or peer");
        _uploading_images = false;
        _pending_image_paths.clear();
        return;
    }

    if (err != ErrorCodes::SUCCESS || results.isEmpty())
    {
        LOGE(LogModule::Ui, "onImageUploadFinished error={} results={}", static_cast<int>(err),
             results.size());
        _uploading_images = false;
        _pending_image_paths.clear();
        return;
    }

    LOGI(LogModule::Ui, "onImageUploadFinished success count={}", results.size());

    QJsonObject image_obj;
    QJsonArray image_array;
    for (const auto &result : results)
    {
        const QString local_path = result.first;
        const QString url = result.second;
        if (url.isEmpty())
        {
            LOGW(LogModule::Ui, "onImageUploadFinished empty url for path={}",
                 local_path.toStdString());
            continue;
        }
        QUuid uuid = QUuid::createUuid();
        QString uuid_string = uuid.toString();
        QPixmap pix = loadImageScaled(local_path);
        QJsonObject obj;
        obj["msgid"] = uuid_string;
        obj["url"] = url;
        obj["width"] = pix.isNull() ? 0 : pix.width();
        obj["height"] = pix.isNull() ? 0 : pix.height();
        obj["size"] = QFileInfo(local_path).size();
        obj["filename"] = QFileInfo(local_path).fileName();
        image_array.append(obj);

        auto img_msg = std::make_shared<TextChatData>(
            uuid_string, QString(), self_info->uid, _peer->uid(), ChatMsgType::Image, url);
        emit sig_append_send_chat_msg(img_msg);
    }

    if (!image_array.isEmpty())
    {
        image_obj["image_array"] = image_array;
        image_obj["fromuid"] = self_info->uid;
        image_obj["touid"] = _peer->uid();
        QJsonDocument doc(image_obj);
        QByteArray json_data = doc.toJson(QJsonDocument::Compact);
        LOGI(LogModule::Ui, "sending image chat msg peer={} count={}", _peer->uid(),
             image_array.size());
        emit TcpMgr::getInstance().sig_send_data(ReqId::ID_IMAGE_CHAT_MSG_REQ, json_data);
    }

    TcpMgr::getInstance().notifyFileTransferDone(self_info->uid);

    _uploading_images = false;
    _pending_image_paths.clear();
}

void ChatPage::onImageDownloadFinished(const QString &url, const QPixmap &pixmap)
{
    auto it = _image_bubbles.find(url);
    if (it == _image_bubbles.end())
    {
        return;
    }
    if (pixmap.isNull())
    {
        LOGE(LogModule::Ui, "onImageDownloadFailed url={}", url.toStdString());
        _image_bubbles.erase(it);
        return;
    }
    LOGI(LogModule::Ui, "onImageDownloadFinished url={} size={}x{}", url.toStdString(),
         pixmap.width(), pixmap.height());
    for (PictureBubble *bubble : it.value())
    {
        if (bubble)
        {
            bubble->setImage(pixmap);
        }
    }
    _image_bubbles.erase(it);
}

void ChatPage::setFriendEntry(std::shared_ptr<FriendListEntry> peer)
{
    _peer = std::move(peer);
    LOGI(LogModule::Ui, "setFriendEntry uid={} name={}", _peer->uid(),
         _peer->listDisplayName().toStdString());
    ui->title_label->setText(_peer->listDisplayName());
    ui->chat_data_list->removeAllItem();
    for (const auto &msg : _peer->chat_msgs)
    {
        appendChatMsg(msg);
    }
}

void ChatPage::appendChatMsg(std::shared_ptr<TextChatData> msg)
{
    auto self_info = UserMgr::getInstance().getSelfProfile();
    if (!self_info)
    {
        return;
    }
    ChatRole role;
    if (msg->_from_uid == self_info->uid)
    {
        role = ChatRole::SELF;
        ChatItemBase *pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->loginName);
        pChatItem->setUserIcon(QPixmap(self_info->icon));
        QWidget *pBubble = nullptr;
        if (msg->_msg_type == ChatMsgType::Image)
        {
            const bool is_local = !msg->_url.startsWith(QStringLiteral("http"), Qt::CaseInsensitive);
            appendImageBubble(msg->_url, role, is_local);
        }
        else
        {
            pBubble = new TextBubble(role, msg->_msg_content);
        }
        if (pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->chat_data_list->appendChatItem(pChatItem);
        }
        else
        {
            delete pChatItem;
        }
    }
    else
    {
        role = ChatRole::OTHER;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::getInstance().getFriendById(msg->_from_uid);
        QString showName = friend_info ? friend_info->listDisplayName() : QString();
        QString showIcon = friend_info ? friend_info->profile.icon : QString();
        if (showName.isEmpty() && _peer && _peer->uid() == msg->_from_uid)
        {
            showName = _peer->listDisplayName();
            showIcon = _peer->profile.icon;
        }
        if (showName.isEmpty())
        {
            delete pChatItem;
            return;
        }
        pChatItem->setUserName(showName);
        pChatItem->setUserIcon(QPixmap(showIcon));
        QWidget *pBubble = nullptr;
        if (msg->_msg_type == ChatMsgType::Image)
        {
            appendImageBubble(msg->_url, role, false);
            _pending_download_urls.enqueue(msg->_url);
        }
        else
        {
            pBubble = new TextBubble(role, msg->_msg_content);
        }
        if (pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->chat_data_list->appendChatItem(pChatItem);
        }
        else
        {
            delete pChatItem;
        }
    }
}
