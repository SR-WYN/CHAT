#pragma once
#include <QLabel>
#include <QMouseEvent>

class ClickedOnceLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedOnceLabel(QWidget *parent = nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
signals:
    void clicked(QString); 
};