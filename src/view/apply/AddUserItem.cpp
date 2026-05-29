#include "AddUserItem.h"
#include "ui_AddUserItem.h"

AddUserItem::AddUserItem(QWidget *parent) : ListItemBase(parent), ui(new Ui::AddUserItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::ADD_TIP_USER_ITEM);
}

AddUserItem::~AddUserItem()
{
    delete ui;
}

QSize AddUserItem::sizeHint() const
{
    return QSize(250, 70);
}