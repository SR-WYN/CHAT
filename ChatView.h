#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class ChatView : public QWidget
{
    Q_OBJECT

public:
    ChatView(QWidget *parent = nullptr);
    void appendChatItem(QWidget *item);
    void prependChatItem(QWidget *item);
    void insertChatItem(QWidget *before, QWidget *item);
    void removeAllItem();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void initStyleSheet();

    QVBoxLayout *_chat_list_layout; // 聊天项垂直容器，底部包含一个弹性占位控件
    QScrollArea *_scroll_area;      // 聊天内容滚动区域
    bool _is_append;                // 标记本次 rangeChanged 是否由“追加消息”触发

private slots:
    void slot_vscrollbar_moved(int min, int max);
};