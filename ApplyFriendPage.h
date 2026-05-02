#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ApplyFriendPage; }
QT_END_NAMESPACE

class AddFriendApply;

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage() override;
    void addNewApply(std::shared_ptr<AddFriendApply> apply);
    void loadApplyList();

private:
    Ui::ApplyFriendPage *ui;
private slots:
    void slot_auth_rsp();
};

#endif // APPLYFRIENDPAGE_H
