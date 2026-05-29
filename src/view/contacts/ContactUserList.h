#pragma once

#include <QListWidget>
#include <memory>

class QListWidgetItem;
class QEvent;
class ConUserItem;

struct AuthAcceptedPeer;struct FriendListEntry;
struct UserProfile;

class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    explicit ContactUserList(QWidget *parent = nullptr);
    void showRedPoint(bool bshow = true);

private:
    void addContactUserList();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_add_auth_firend(std::shared_ptr<AuthAcceptedPeer> peer);
    void slot_auth_rsp(std::shared_ptr<AuthAcceptedPeer> peer);

signals:
    void sig_loading_contact_user();
    void sig_switch_apply_friend_page();
    void sig_switch_friend_info_page(std::shared_ptr<FriendListEntry> entry);

private:
    ConUserItem *_add_friend_item;
    QListWidgetItem *_groupitem;
    bool _load_pending;
};
