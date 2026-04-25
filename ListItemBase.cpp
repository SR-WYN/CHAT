#include "ListItemBase.h"

ListItemBase::ListItemBase(QWidget *parent) : QWidget(parent)
{
}

void ListItemBase::setItemType(ListItemType item_type)
{
    _item_type = item_type;
}
ListItemType ListItemBase::getItemType() const
{
    return _item_type;
}