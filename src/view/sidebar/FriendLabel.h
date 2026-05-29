#ifndef FRIENDLABEL_H
#define FRIENDLABEL_H

#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FriendLabel;
}
QT_END_NAMESPACE

class FriendLabel : public QFrame
{
    Q_OBJECT

public:
    explicit FriendLabel(QWidget *parent = nullptr);
    ~FriendLabel() override;
    void setText(QString text);
    int width();
    int height();
    QString text();

private:
    Ui::FriendLabel *ui;
    QString _text;
    int _width = 0;
    int _height = 0;
public slots:
    void slot_close();
signals:
    void sig_close(QString);
};

#endif // FRIENDLABEL_H
