#pragma once
#include "BubbleFrame.h"

class PictureBubble : public BubbleFrame
{
    Q_OBJECT
public:
    PictureBubble(const QPixmap &picture,ChatRole role,QWidget *parent = nullptr);
};