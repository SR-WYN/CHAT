#ifndef APPLYFRIENDITEM_H
#define APPLYFRIENDITEM_H

#include "ListItemBase.h"
#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ApplyFriendItem;
}
QT_END_NAMESPACE

struct PendingFriendApplyRow;

class ApplyFriendItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();
    void SetInfo(std::shared_ptr<PendingFriendApplyRow> apply_row);
    void ShowAddBtn(bool bshow);
    QSize sizeHint() const override
    {
        return QSize(250, 80);
    }
    int GetUid();

private:
    Ui::ApplyFriendItem *ui;
    std::shared_ptr<PendingFriendApplyRow> _apply_row;
    bool _added;

signals:
    void sig_auth_friend(std::shared_ptr<PendingFriendApplyRow> apply_row);
};

#endif // APPLYFRIENDITEM_H
