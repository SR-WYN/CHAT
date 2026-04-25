#pragma once
#include "global.h"
#include <QWidget>
#include <qtmetamacros.h>
#include <qwidget.h>

class ListItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent = nullptr);
    void setItemType(ListItemType item_type);
    ListItemType getItemType() const;

private:
    ListItemType _item_type;
public slots:

signals:
};