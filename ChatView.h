#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

class ChatView : public QWidget
{
    Q_OBJECT
public:
    ChatView(QWidget *parent = nullptr);
    void appendChatItem(QWidget *item);
    void prependChatItem(QWidget *item);
    void insertChatItem(QWidget *before, QWidget *item);
protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void initStyleSheet();
    QVBoxLayout *_vlayout;
    QScrollArea *_scrollArea;
    bool _isAppend;
private slots:
    void slot_vscrollbar_moved(int min,int max);
};