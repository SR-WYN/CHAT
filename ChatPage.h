#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ChatPage;
}
QT_END_NAMESPACE

class UserInfo;
class TextChatData;

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage() override;
    void setUserInfo(std::shared_ptr<UserInfo> user_info);
    void appendChatMsg(std::shared_ptr<TextChatData> msg);
protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    Ui::ChatPage *ui;
    std::shared_ptr<UserInfo> _user_info;
    
private slots:
    void on_send_btn_clicked();
signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};

#endif // CHATPAGE_H
