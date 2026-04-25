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

class ChatUserWidget : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWidget(QWidget *parent = nullptr);
    ~ChatUserWidget() override;

    QSize sizeHint() const override;
    void setInfo(QString name,QString head,QString msg);

private:
    Ui::ChatUserWidget *ui;
    QString _name;
    QString _head;
    QString _msg;
};

#endif // CHATUSERWIDGET_H
