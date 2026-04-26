#include "ClickedLabel.h"
#include "QMouseEvent"
#include "global.h"
#include <qevent.h>
#include <qtmetamacros.h>

ClickedLabel::ClickedLabel(QWidget *parent) : QLabel(parent), _cur_state(ClickLabelState::NORMAL)
{
    setCursor(Qt::PointingHandCursor);
}

ClickedLabel::~ClickedLabel()
{
}

void ClickedLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        if (_cur_state == ClickLabelState::NORMAL)
        {
            setProperty("state", _normal_press);
        }
        else 
        {
            setProperty("state", _select_press);
        }
        repolish(this);
        update();
        return;
    }
    QLabel::mousePressEvent(ev);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        if (_cur_state == ClickLabelState::NORMAL)
        {
            _cur_state = ClickLabelState::SELECTED;
            setProperty("state", _select_hover);
        }
        else 
        {
            _cur_state = ClickLabelState::NORMAL;
            setProperty("state", _normal_hover);
        }
        repolish(this);
        update();
        emit sig_label_clicked();
        return;
    }
    QLabel::mouseReleaseEvent(ev);
}

void ClickedLabel::setState(QString normal, QString hover, QString press,
                              QString select_normal, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;
    _select_normal = select_normal;
    _select_hover = select_hover;
    _select_press = select_press;

    setProperty("state",normal);
    repolish(this);
}

ClickLabelState ClickedLabel::getCurState() const
{
    return _cur_state;
}

void ClickedLabel::enterEvent(QEnterEvent *event)
{
    if (_cur_state == ClickLabelState::NORMAL)
    {
        setProperty("state", _normal_hover);
    }
    else
    {
        setProperty("state", _select_hover);
    }
    repolish(this);
    update();
    QLabel::enterEvent(event);
}

void ClickedLabel::leaveEvent(QEvent *event)
{
    if (_cur_state == ClickLabelState::NORMAL)
    {
        setProperty("state", _normal);
    }
    else
    {
        setProperty("state", _select_normal);
    }
    repolish(this);
    update();
    QLabel::leaveEvent(event);
}