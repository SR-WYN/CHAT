#pragma once

#include <QFrame>
#include "global.h"
#include <QHBoxLayout>
#include <QWidget>

class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role,QWidget *parent = nullptr);
    void setWidget(QWidget *w);
protected:
    void paintEvent(QPaintEvent *e);
private:
    QHBoxLayout *_p_hlayout;
    ChatRole _role;
    int _margin;
};