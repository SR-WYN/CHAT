#ifndef GROUPTIPITEM_H
#define GROUPTIPITEM_H

#include "ListItemBase.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class GroupTipItem; }
QT_END_NAMESPACE

class GroupTipItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit GroupTipItem(QWidget *parent = nullptr);
    ~GroupTipItem() override;
    QSize sizeHint() const override;
    void setGroupTip(QString str);


private:
    Ui::GroupTipItem *ui;
    QString _tip;
};

#endif // GROUPTIPITEM_H
