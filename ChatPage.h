#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ChatPage;
}
QT_END_NAMESPACE

struct FriendListEntry;
struct TextChatData;

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage() override;
    void setFriendEntry(std::shared_ptr<FriendListEntry> peer);
    void appendChatMsg(std::shared_ptr<TextChatData> msg);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::ChatPage *ui;
    std::shared_ptr<FriendListEntry> _peer;

private slots:
    void on_send_btn_clicked();

signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};

#endif // CHATPAGE_H
