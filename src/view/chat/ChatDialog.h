#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <qlistwidget.h>
#include "AnimatedStateWidget.h"
#include "UserData.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class ChatDialog;
}
QT_END_NAMESPACE

struct FriendApplyNotify;
struct AuthAcceptedPeer;
struct UserProfile;
struct FriendListEntry;
struct TextChatData;
struct TextChatMsg;

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog() override;
    void addChatUserList();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void showSearch(bool bsearch);
    void addLabelGroup(AnimatedStateWidget *label);
    void clearLabelState(AnimatedStateWidget *active);
    void handleGlobalMousePress(QMouseEvent *mouse_event);
    void setSelectChatItem(int uid = 0);
    void setSelectChatPage(int uid = 0);
    void loadMoreChatUser();
    void loadMoreContactUser();
    void updateChatMsg(const std::vector<std::shared_ptr<TextChatData>> &msg_vec);
    void refreshChatListFromMemory();
    void refreshChatListItem(int peer_uid);
    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<AnimatedStateWidget *> _label_list;
    QMap<int, QListWidgetItem *> _chat_item_added;
    int _cur_chat_uid;
    QWidget *_last_widget;

private slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString &str);
    void slot_friend_apply(std::shared_ptr<FriendApplyNotify> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthAcceptedPeer> peer);
    void slot_auth_rsp(std::shared_ptr<AuthAcceptedPeer> peer);
    void slot_jump_chat_item(std::shared_ptr<UserProfile> profile);
    void slot_loading_contact_user();
    void slot_friend_info_page(std::shared_ptr<FriendListEntry> entry);
    void slot_jump_chat_item_from_infopage(std::shared_ptr<FriendListEntry> entry);
    void slot_item_clicked(QListWidgetItem *item);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg_ptr);
    void slot_chat_history(int peer_uid, std::vector<std::shared_ptr<TextChatData>> msgs);
    void requestChatHistory(int peer_uid);
    void slot_side_head_clicked();
    void slot_back_from_self_info();

signals:
    void sig_switch_login();
    void sig_switch_self_info();
};

#endif // CHATDIALOG_H
