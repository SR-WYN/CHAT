#include "CustomsizeEdit.h"
#include <qlineedit.h>

CustomsizeEdit::CustomsizeEdit(QWidget *parent) : QLineEdit(parent), _max_len(32767)
{
    setMaxLength(_max_len);
}

CustomsizeEdit::~CustomsizeEdit()
{
}

void CustomsizeEdit::setMaxLength(int maxLen)
{
    QLineEdit::setMaxLength(maxLen);
}

void CustomsizeEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    emit sig_focus_out();
}
