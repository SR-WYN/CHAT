#ifndef CUSTOMSIZEEDIT_H
#define CUSTOMSIZEEDIT_H

#include <QLineEdit>
#include <QObject>
#include <QEvent>

class CustomsizeEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit CustomsizeEdit(QWidget *parent = nullptr);
    ~CustomsizeEdit() override;
    void setMaxLength(int maxLen);
protected:
    void focusOutEvent(QFocusEvent *event) override;
private:
    int _max_len;
signals:
    void sig_focus_out();
};

#endif // CUSTOMSIZEEDIT_H
