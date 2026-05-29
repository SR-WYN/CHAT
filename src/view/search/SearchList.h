#pragma once
#include "StatusDialog.h"
#include <QEvent>
#include <QListWidget>
#include <QScrollBar>
#include <QWheelEvent>
#include <memory>
#include <qtmetamacros.h>

struct UserProfile;

class SearchList : public QListWidget
{
    Q_OBJECT
public:
    SearchList(QWidget *parent = nullptr);
    void closeFindDlg();
    void setSearchEdit(QWidget *edit);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void waitPending(bool pending = true);
    bool _send_pending;
    void addTipItem();
    StatusDialog *_find_dlg;
    QWidget *_search_edit;
    StatusDialog *_loadingDialog;

private slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_user_search(std::shared_ptr<UserProfile> profile);

signals:
    void sig_jump_chat_item(std::shared_ptr<UserProfile> profile);
};
