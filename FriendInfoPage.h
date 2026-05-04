#ifndef FRIENDINFOPAGE_H
#define FRIENDINFOPAGE_H

#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class FriendInfoPage; }
QT_END_NAMESPACE

class UserInfo;

class FriendInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit FriendInfoPage(QWidget *parent = nullptr);
    ~FriendInfoPage() override;
    void setInfo(std::shared_ptr<UserInfo> user_info);

private:
    Ui::FriendInfoPage *ui;
    std::shared_ptr<UserInfo> _user_info;
private slots:
    void on_msg_chat_button_clicked();
signals:
    void sig_jump_chat_item(std::shared_ptr<UserInfo> si);
};

#endif // FRIENDINFOPAGE_H
