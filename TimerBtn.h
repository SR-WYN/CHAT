#pragma once

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
    Q_OBJECT
public:
    TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();
    void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QTimer *_timer;
    int _counter;
};