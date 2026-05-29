#include "ChatView.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>
#include <QTimer>
#include <qboxlayout.h>

ChatView::ChatView(QWidget *parent)
    : QWidget(parent), _chat_list_layout(nullptr), _scroll_area(nullptr), _is_append(false)
{
    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setContentsMargins(0, 0, 0, 0);

    _scroll_area = new QScrollArea();
    _scroll_area->setObjectName("chat_area");
    main_layout->addWidget(_scroll_area);

    QWidget *chat_background = new QWidget(this);
    chat_background->setObjectName("chat_background");
    chat_background->setAutoFillBackground(true);

    _chat_list_layout = new QVBoxLayout();
    // 底部弹性控件用于把聊天消息“顶到上方”，保证视觉上从上往下排列。
    _chat_list_layout->addWidget(new QWidget(), 100000);
    chat_background->setLayout(_chat_list_layout);
    _scroll_area->setWidget(chat_background);

    _scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *vertical_scrollbar = _scroll_area->verticalScrollBar();
    connect(vertical_scrollbar, &QScrollBar::rangeChanged, this, &ChatView::slot_vscrollbar_moved);

    QHBoxLayout *scroll_layout = new QHBoxLayout();
    scroll_layout->addWidget(vertical_scrollbar, 0, Qt::AlignRight);
    scroll_layout->setContentsMargins(0, 0, 0, 0);
    _scroll_area->setLayout(scroll_layout);
    vertical_scrollbar->setHidden(true);
    _scroll_area->setWidgetResizable(true);
    _scroll_area->installEventFilter(this);
    initStyleSheet();
}

void ChatView::slot_vscrollbar_moved(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);

    if (_is_append)
    {
        QScrollBar *vertical_scrollbar = _scroll_area->verticalScrollBar();
        vertical_scrollbar->setSliderPosition(vertical_scrollbar->maximum());
        // 等布局和滚动范围稳定后再清理标记，避免连续追加时丢失“自动到底部”。
        QTimer::singleShot(500, [this]() {
            _is_append = false;
        });
    }
}

void ChatView::initStyleSheet()
{
}

void ChatView::appendChatItem(QWidget *item)
{
    _chat_list_layout->insertWidget(_chat_list_layout->count() - 1, item);
    // 通过 rangeChanged 回调把滚动条同步到最底部，模拟常见聊天窗口行为。
    _is_append = true;
}

void ChatView::prependChatItem(QWidget *item)
{
}

void ChatView::insertChatItem(QWidget *before, QWidget *item)
{
}

bool ChatView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter && obj == _scroll_area)
    {
        // 鼠标进入时仅在存在可滚动内容时显示滚动条。
        _scroll_area->verticalScrollBar()->setHidden(_scroll_area->verticalScrollBar()->maximum() ==
                                                     0);
    }
    else if (event->type() == QEvent::Leave && obj == _scroll_area)
    {
        // 鼠标离开时隐藏滚动条，保持聊天视图简洁。
        _scroll_area->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(obj, event);
}

void ChatView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    // 让样式表背景在自绘 QWidget 上生效。
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ChatView::removeAllItem()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(_scroll_area->widget()->layout());
    int count = layout->count();
    for (int i = 0;i<count-1;i++)
    {
        QLayoutItem *item =layout->takeAt(0);
        if (item)
        {
            if (QWidget *widget = item->widget())
            {
                delete widget;
            }
            delete item;
        }
    }
}