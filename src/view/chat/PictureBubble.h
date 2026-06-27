#pragma once
#include "BubbleFrame.h"
#include <QSize>

class QLabel;

class PictureBubble : public BubbleFrame
{
    Q_OBJECT
public:
    PictureBubble(const QPixmap &picture, ChatRole role, QWidget *parent = nullptr,
                  const QSize &max_size = QSize(300, 400));

    void setImage(const QPixmap &picture);

private:
    QLabel *_image_label = nullptr;
    QSize _max_size;
};
