#ifndef CHATUSERWIDGET_H
#define CHATUSERWIDGET_H

#include "ListItemBase.h"
#include <QWidget>
#include <qobject.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ChatUserWidget;
}
QT_END_NAMESPACE

class UserInfo;
class TextChatData;

class ChatUserWidget : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWidget(QWidget *parent = nullptr);
    ~ChatUserWidget() override;

    QSize sizeHint() const override;
    void setInfo(std::shared_ptr<UserInfo> user_info);
    std::shared_ptr<UserInfo> getUserInfo() const;
    void updateLastMsg(const std::vector<std::shared_ptr<TextChatData>> &msg_vec);
private:
    Ui::ChatUserWidget *ui;
    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATUSERWIDGET_H
