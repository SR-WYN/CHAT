#include "ChatView.h"
#include <QEvent>
#include <QScrollBar>
#include <QTimer>
#include <QStyleOption>
#include <QPainter>

ChatView::ChatView(QWidget *parent) : QWidget(parent), _isAppend(false)
{
    QVBoxLayout *p_main_layout = new QVBoxLayout();
    this->setLayout(p_main_layout);
    p_main_layout->setContentsMargins(0, 0, 0, 0);

    _scrollArea = new QScrollArea();
    _scrollArea->setObjectName("chat_area");
    p_main_layout->addWidget(_scrollArea);

    QWidget *w = new QWidget(this);
    w->setObjectName("chat_background");
    w->setAutoFillBackground(true);

    QVBoxLayout *p_h_layout_1 = new QVBoxLayout();
    p_h_layout_1->addWidget(new QWidget(), 100000);
    w->setLayout(p_h_layout_1);
    _scrollArea->setWidget(w);

    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *p_vscrollbar = _scrollArea->verticalScrollBar();
    connect(p_vscrollbar, &QScrollBar::rangeChanged, this, &ChatView::slot_vscrollbar_moved);

    QHBoxLayout *p_h_layout_2 = new QHBoxLayout();
    p_h_layout_2->addWidget(p_vscrollbar, 0, Qt::AlignRight);
    p_h_layout_2->setContentsMargins(0, 0, 0, 0);
    _scrollArea->setLayout(p_h_layout_2);
    p_vscrollbar->setHidden(true);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->installEventFilter(this);
    initStyleSheet();
}

void ChatView::slot_vscrollbar_moved(int min, int max)
{
    if (_isAppend)
    {
        QScrollBar *p_vscrollbar = _scrollArea->verticalScrollBar();
        p_vscrollbar->setSliderPosition(p_vscrollbar->maximum());
        QTimer::singleShot(500, [this]() {
            _isAppend = false;
        });
    }
}

void ChatView::initStyleSheet()
{
}

void ChatView::appendChatItem(QWidget *item)
{
    QVBoxLayout *vl = qobject_cast<QVBoxLayout *>(_scrollArea->widget()->layout());
    vl->insertWidget(vl->count() - 1, item);
    _isAppend = true;
}

void ChatView::prependChatItem(QWidget *item)
{
}

void ChatView::insertChatItem(QWidget *before, QWidget *item)
{
}

bool ChatView::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Enter && o == _scrollArea)
    {
        _scrollArea->verticalScrollBar()->setHidden(_scrollArea->verticalScrollBar()->maximum() ==
                                                    0);
    }
    else if (e->type() == QEvent::Leave && o == _scrollArea)
    {
        _scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);
}

void ChatView::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}