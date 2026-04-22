#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H

#include <QLabel>
#include <QObject>
#include "global.h"

class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget *parent = nullptr);
    ~ClickedLabel() override;
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    void setState(QString normal = "", QString hover = "", QString press = "",
                  QString select_normal = "", QString select_hover = "", QString select_press = "");
    ClickLabelState getCurState() const;
private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;
    QString _select_normal;
    QString _select_hover;
    QString _select_press;

    ClickLabelState _cur_state;
signals:
    void sig_label_clicked();
};

#endif // CLICKEDLABEL_H
