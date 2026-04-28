#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ApplyFriendPage; }
QT_END_NAMESPACE

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage() override;


private:
    Ui::ApplyFriendPage *ui;
};

#endif // APPLYFRIENDPAGE_H
