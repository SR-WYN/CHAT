#ifndef ADDUSERITEM_H
#define ADDUSERITEM_H

#include "ListItemBase.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class AddUserItem;
}
QT_END_NAMESPACE

class AddUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit AddUserItem(QWidget *parent = nullptr);
    ~AddUserItem() override;
    QSize sizeHint() const override;
private:
    Ui::AddUserItem *ui;
};

#endif // ADDUSERITEM_H
