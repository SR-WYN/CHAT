#include "PictureBubble.h"
#include <QLabel>

PictureBubble::PictureBubble(const QPixmap &picture, ChatRole role, QWidget *parent,
                             const QSize &max_size)
    : BubbleFrame(role, parent), _max_size(max_size)
{
    _image_label = new QLabel();
    _image_label->setScaledContents(true);
    setImage(picture);
    this->setWidget(_image_label);
}

void PictureBubble::setImage(const QPixmap &picture)
{
    if (!_image_label)
    {
        return;
    }
    QPixmap pix = picture.scaled(_max_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _image_label->setPixmap(pix);

    int left_margin = this->layout()->contentsMargins().left();
    int right_margin = this->layout()->contentsMargins().right();
    int v_margin = this->layout()->contentsMargins().bottom();
    setFixedSize(pix.width() + left_margin + right_margin, pix.height() + v_margin * 2);
}
