#ifndef AUTHENFRIEND_H
#define AUTHENFRIEND_H

#include <QDialog>
#include "global.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AuthenFriend; }
QT_END_NAMESPACE

class ClickedLabel;
class FriendLabel;
class SearchInfo;
class ApplyFriendItem;
class ApplyInfo;
class AuthenFriend : public QDialog
{
    Q_OBJECT

public:
    explicit AuthenFriend(QWidget *parent = nullptr);
    ~AuthenFriend();
    void initTipLbs();
    void addTipLbs(ClickedLabel *, QPoint cur_point, QPoint &next_point, int text_width,
                   int text_height);
    bool eventFilter(QObject *obj, QEvent *event);
    void setApplyInfo(std::shared_ptr<ApplyInfo> apply_info);
private:
    Ui::AuthenFriend *ui;
    void resetLabels();

    //已经创建好的标签
    QMap<QString, ClickedLabel *> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;
    //用来在输入框显示添加新好友的标签
    QMap<QString, FriendLabel *> _friend_labels;
    std::vector<QString> _friend_label_keys;
    void addLabel(QString name);
    std::vector<QString> _tip_data;
    QPoint _tip_cur_point;
    std::shared_ptr<ApplyInfo> _apply_info;

private slots:
    void slot_more_lb_clicked();
    void slot_lb_ed_return_pressed();
    void slot_remove_friend_label(QString);
    void slot_change_friend_label_by_tip(QString, ClickLabelState);
    void slot_lb_ed_text_changed(const QString &text);
    void slot_lb_ed_editing_finished();
    void slot_add_friend_label_by_click_tip(QString text);
    void slot_sure_btn_clicked();
    void slot_cancel_btn_clicked();
};

#endif // AUTHENFRIEND_H
