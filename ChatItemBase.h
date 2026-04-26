#pragma once
#include "global.h"
#include <QLabel>
#include <QWidget>

class BubbleFrame;

class ChatItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);
    void setUserName(const QString &name);
    void setUserIcon(const QPixmap &icon);
    void setWidget(QWidget *w);

private:
    ChatRole _role;
    QLabel *_p_name_label;
    QLabel *_p_icon_label;
    QWidget *_p_bubble;
};