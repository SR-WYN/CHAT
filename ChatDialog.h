#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"
#include "StateWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatDialog; }
QT_END_NAMESPACE

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog() override;
    void addChatUserList();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    void showSearch(bool bsearch);
    void addLabelGroup(StateWidget *label);
    void clearLabelState(StateWidget *label);
    void handleGlobalMousePress(QMouseEvent *mouse_event);
    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<StateWidget *> _label_list;
private slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString &str);
};

#endif // CHATDIALOG_H
