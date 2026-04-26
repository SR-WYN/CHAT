#include "ChatItemBase.h"
#include <qfont.h>
#include <qgridlayout.h>

ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent) : QWidget(parent), _role(role)
{
    _p_name_label = new QLabel();
    _p_name_label->setObjectName("chat_user_name");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    _p_name_label->setFont(font);
    _p_name_label->setFixedHeight(20);

    _p_icon_label = new QLabel();
    _p_icon_label->setScaledContents(true);
    _p_icon_label->setFixedSize(42, 42);

    _p_bubble = new QWidget();

    QGridLayout *grid_layout = new QGridLayout();
    grid_layout->setVerticalSpacing(3);
    grid_layout->setHorizontalSpacing(3);
    grid_layout->setContentsMargins(3, 3, 3, 3);

    QSpacerItem *pSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    if (_role == ChatRole::SELF)
    {
        _p_name_label->setContentsMargins(0, 0, 8, 0);
        _p_name_label->setAlignment(Qt::AlignRight);
        grid_layout->addWidget(_p_name_label, 0, 1, 1, 1);
        grid_layout->addWidget(_p_icon_label, 0, 2, 2, 1, Qt::AlignTop);
        grid_layout->addItem(pSpacer, 1, 0, 1, 1);
        grid_layout->addWidget(_p_bubble, 1, 1, 1, 1);
        grid_layout->setColumnStretch(0, 2);
        grid_layout->setColumnStretch(1, 3);
    }
    else
    {
        _p_name_label->setContentsMargins(8, 0, 0, 0);
        _p_name_label->setAlignment(Qt::AlignLeft);
        grid_layout->addWidget(_p_icon_label, 0, 0, 2, 1, Qt::AlignTop);
        grid_layout->addWidget(_p_name_label, 0, 1, 1, 1);
        grid_layout->addWidget(_p_bubble, 1, 1, 1, 1);
        grid_layout->addItem(pSpacer, 2, 2, 1, 1);
        grid_layout->setColumnStretch(1, 3);
        grid_layout->setColumnStretch(2, 2);
    }
    this->setLayout(grid_layout);
}

void ChatItemBase::setUserName(const QString &name)
{
    _p_name_label->setText(name);
}

void ChatItemBase::setUserIcon(const QPixmap &icon)
{
    _p_icon_label->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w)
{
   QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
   pGLayout->replaceWidget(_p_bubble, w);
   delete _p_bubble;
   _p_bubble = w;
}