#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include "ListItemBase.h"
#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ConUserItem;
}
QT_END_NAMESPACE

struct AuthAcceptedPeer;
struct FriendListEntry;
struct UserProfile;

class ConUserItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();
    QSize sizeHint() const override;
    void setInfo(std::shared_ptr<AuthAcceptedPeer> peer);
    void setInfo(UserProfile profile);
    void showRedPoint(bool show = false);
    std::shared_ptr<FriendListEntry> getFriendEntry() const;

private:
    void refreshUi();

    Ui::ConUserItem *ui;
    std::shared_ptr<FriendListEntry> _entry;
};

#endif // CONUSERITEM_H
