#include "StateWidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOption>

StateWidget::StateWidget(QWidget *parent) : QWidget(parent), _curstate(ClickLabelState::NORMAL)
{
    setCursor(Qt::PointingHandCursor);
    addRedPoint();
    setState("normal", "hover", "pressed", "selected_normal", "selected_hover", "selected_pressed");
    setProperty("state", _normal);
}

void StateWidget::addRedPoint()
{
    _red_point = new QLabel(this);
    _red_point->setObjectName("red_point");
    const QSize badgeSize(50, 50);
    QPixmap pm(QStringLiteral(":/res/red_point.png"));
    if (!pm.isNull())
    {
        _red_point->setPixmap(
            pm.scaled(badgeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    _red_point->setFixedSize(badgeSize);
    _red_point->setAlignment(Qt::AlignCenter);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(_red_point, 0, Qt::AlignTop | Qt::AlignRight);
    _red_point->setVisible(false);
}

void StateWidget::setState(QString normal, QString hover, QString press, QString select,
                           QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty("state", normal);
    repolish(this);
}
ClickLabelState StateWidget::getCurState() const
{
    return _curstate;
}

void StateWidget::clearState()
{
    _curstate = ClickLabelState::NORMAL;
    setProperty("state", _normal);
    repolish(this);
    update();
}

void StateWidget::setSelected(bool bselected)
{
    if (bselected)
    {
        _curstate = ClickLabelState::SELECTED;
        setProperty("state", _selected);
        repolish(this);
        update();
        return;
    }

    _curstate = ClickLabelState::NORMAL;
    setProperty("state", _normal);
    repolish(this);
    update();
    return;
}

void StateWidget::showRedPoint(bool bshow)
{
    _red_point->setVisible(bshow);
}

void StateWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    return;
}

void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (_curstate == ClickLabelState::NORMAL)
        {
            setProperty("state", _normal_press);
        }
        else
        {
            setProperty("state", _selected_press);
        }
        repolish(this);
        update();
        return;
    }
    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // 切换状态
        if (_curstate == ClickLabelState::NORMAL)
        {
            _curstate = ClickLabelState::SELECTED;
            setProperty("state", _selected_hover);
        }
        else
        {
            _curstate = ClickLabelState::NORMAL;
            setProperty("state", _normal_hover);
        }
        repolish(this);
        update();
        emit clicked();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void StateWidget::enterEvent(QEnterEvent *event)
{
    if (_curstate == ClickLabelState::NORMAL)
    {
        setProperty("state", _normal_hover);
    }
    else
    {
        setProperty("state", _selected_hover);
    }
    repolish(this);
    update();
    QWidget::enterEvent(event);
}

void StateWidget::leaveEvent(QEvent *event)
{
    if (_curstate == ClickLabelState::NORMAL)
    {
        setProperty("state", _normal);
    }
    else
    {
        setProperty("state", _selected);
    }
    repolish(this);
    update();
    QWidget::leaveEvent(event);
}