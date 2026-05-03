#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <qlistwidget.h>
#include "global.h"
#include "StateWidget.h"
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
    void addLabelGroup(StateWidget *label);
    void clearLabelState(StateWidget *label);
    void handleGlobalMousePress(QMouseEvent *mouse_event);
    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<StateWidget *> _label_list;
    QMap<int,QListWidgetItem*> _chat_item_added;
private slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString &str);
    void slot_friend_apply(std::shared_ptr<AddFriendApply> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
};

#endif // CHATDIALOG_H
