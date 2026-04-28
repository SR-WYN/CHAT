#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include "ClickedLabel.h"
#include "FriendLabel.h"
#include "UserData.h"
#include "global.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ApplyFriend;
}
QT_END_NAMESPACE

class SearchInfo;

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    void initTipLbs();
    void addTipLbs(ClickedLabel *, QPoint cur_point, QPoint &next_point, int text_width,
                   int text_height);
    bool eventFilter(QObject *obj, QEvent *event);
    void setSearchInfo(std::shared_ptr<SearchInfo> si);

private:
    Ui::ApplyFriend *ui;
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
    std::shared_ptr<SearchInfo> _si;

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

#endif // APPLYFRIEND_H
