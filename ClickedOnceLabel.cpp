#include "ClickedOnceLabel.h"
#include <qnamespace.h>

ClickedOnceLabel::ClickedOnceLabel(QWidget *parent) : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickedOnceLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked(text());
        return;
    }
    QLabel::mouseReleaseEvent(event);
}