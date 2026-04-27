#pragma once
#include "global.h"
#include <QLabel>
#include <QWidget>

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);
    void setState(QString normal = "", QString hover = "", QString press = "",
                  QString select_normal = "", QString select_hover = "", QString select_press = "");
    ClickLabelState getCurState() const;
    void clearState();

    void setSelected(bool bselected);
    void addRedPoint();
    void showRedPoint();

protected:
    void paintEvent(QPaintEvent *event);

    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLabelState _curstate;
    QLabel *_red_point;

signals:
    void clicked(void);
};