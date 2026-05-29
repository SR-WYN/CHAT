#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ApplyFriendPage;
}
QT_END_NAMESPACE

struct FriendApplyNotify;
struct AuthAcceptedPeer;

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage() override;
    void addNewApply(std::shared_ptr<FriendApplyNotify> notify);
    void loadApplyList();

private:
    Ui::ApplyFriendPage *ui;

private slots:
    void slot_auth_rsp(std::shared_ptr<AuthAcceptedPeer> peer);
};

#endif // APPLYFRIENDPAGE_H
