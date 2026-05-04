#pragma once
#include <QListWidget>
#include <QObject>

class ChatUserList : public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent = nullptr);
    ~ChatUserList();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void sig_loading_chat_user();
private:
    bool _load_pending;
};