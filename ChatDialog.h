#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <qlistwidget.h>
#include "global.h"
#include "AnimatedStateWidget.h"
#include "UserData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatDialog; }
QT_END_NAMESPACE

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
    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<AnimatedStateWidget *> _label_list;
    QMap<int,QListWidgetItem*> _chat_item_added;
    int _cur_chat_uid;
    QWidget *_last_widget;
private slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString &str);
    void slot_friend_apply(std::shared_ptr<AddFriendApply> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void slot_loading_contact_user();
    void slot_friend_info_page(std::shared_ptr<UserInfo> user_info);
    void slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info);
    void slot_item_clicked(QListWidgetItem *item);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg_ptr);
};

#endif // CHATDIALOG_H
