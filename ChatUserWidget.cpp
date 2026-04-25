#include "ChatUserWidget.h"
#include "global.h"
#include "ui_ChatUserWidget.h"
#include <qnamespace.h>

ChatUserWidget::ChatUserWidget(QWidget *parent) : ListItemBase(parent), ui(new Ui::ChatUserWidget)
{
    ui->setupUi(this);
    setItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWidget::~ChatUserWidget()
{
    delete ui;
}

QSize ChatUserWidget::sizeHint() const
{
    return QSize(250, 70);
}

void ChatUserWidget::setInfo(QString name, QString head, QString msg)
{
    _name = name;
    _head = head;
    _msg = msg;
    QPixmap pixmap(_head);
    ui->icon_label->setPixmap(
        pixmap.scaled(ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);

    ui->user_name_label->setText(_name);
    ui->user_chat_label->setText(_msg);
}