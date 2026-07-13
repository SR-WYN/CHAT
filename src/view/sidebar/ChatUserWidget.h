#ifndef CHATUSERWIDGET_H
#define CHATUSERWIDGET_H

#include "ListItemBase.h"
#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ChatUserWidget;
}
QT_END_NAMESPACE
namespace Ui
{
class ChatUserWidget;
}
QT_END_NAMESPACE

struct FriendListEntry;
struct TextChatData;

class ChatUserWidget : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWidget(QWidget *parent = nullptr);
    ~ChatUserWidget() override;

    QSize sizeHint() const override;
    void setInfo(std::shared_ptr<FriendListEntry> entry);
    std::shared_ptr<FriendListEntry> getFriendEntry() const;
    void updateLastMsg(const std::vector<std::shared_ptr<TextChatData>> &msg_vec);

private:
    Ui::ChatUserWidget *ui;
    std::shared_ptr<FriendListEntry> _entry;
};

#endif // CHATUSERWIDGET_H
