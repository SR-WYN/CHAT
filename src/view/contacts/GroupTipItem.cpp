#include "GroupTipItem.h"
#include "ListItemBase.h"
#include "ui_GroupTipItem.h"

GroupTipItem::GroupTipItem(QWidget *parent)
    : ListItemBase(parent), _tip(""), ui(new Ui::GroupTipItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::GROUP_TIP_ITEM);
}

GroupTipItem::~GroupTipItem()
{
    delete ui;
}

QSize GroupTipItem::sizeHint() const
{
    return QSize(250, 25);
}

void GroupTipItem::setGroupTip(QString str)
{
    ui->label->setText(str);
}
