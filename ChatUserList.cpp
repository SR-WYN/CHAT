#include "ChatUserList.h"
#include "UserMgr.h"
#include <QScrollBar>
#include <QTimer>
#include <qcoreapplication.h>
#include <qevent.h>
#include <qglobal.h>
#include <qnamespace.h>
#include <qscrollbar.h>

ChatUserList::ChatUserList(QWidget *parent) : QListWidget(parent), _load_pending(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->viewport()->installEventFilter(this);
}

ChatUserList::~ChatUserList()
{
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this->viewport())
    {
        if (event->type() == QEvent::Enter)
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        else if (event->type() == QEvent::Leave)
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    if (watched == this->viewport() && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheel_event = static_cast<QWheelEvent *>(event);
        int num_degress = wheel_event->angleDelta().y() / 8;
        int num_steps = num_degress / 15;
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - num_steps);
        QScrollBar *scrollbar = this->verticalScrollBar();
        int max_scroll_value = scrollbar->maximum();
        int current_value = scrollbar->value();
        if (max_scroll_value - current_value <= 0)
        {
            auto b_loaded = UserMgr::getInstance().isLoadChatFinish();
            if (b_loaded)
            {
                return true;
            }
            if (_load_pending)
            {
                return true;
            }
            _load_pending = true;
            QTimer::singleShot(100, [this]() {
                _load_pending = false;
                // QCoreApplication::quit();
            });
            emit sig_loading_chat_user();
        }
        return true;
    }
    return QListWidget::eventFilter(watched, event);
}