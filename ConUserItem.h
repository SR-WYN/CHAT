#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include "ListItemBase.h"
#include <QWidget>
#include "UserData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ConUserItem; }
QT_END_NAMESPACE

class ConUserItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();
    QSize sizeHint() const override;
    void setInfo(std::shared_ptr<AuthInfo> auth_info);
    void setInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void setInfo(int uid, QString name, QString icon);
    void showRedPoint(bool show = false);
    std::shared_ptr<UserInfo> getUserInfo() const;
private:
    Ui::ConUserItem *ui;
    std::shared_ptr<UserInfo> _info;
};

#endif // CONUSERITEM_H
