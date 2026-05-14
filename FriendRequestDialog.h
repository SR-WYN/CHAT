#ifndef FRIENDREQUESTDIALOG_H
#define FRIENDREQUESTDIALOG_H

#include "global.h"
#include <QDialog>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FriendRequestDialog;
}
QT_END_NAMESPACE

struct UserProfile;
struct PendingFriendApplyRow;
class FriendLabel;
class AnimatedStateWidget;

class FriendRequestDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode
    {
        Apply,
        Auth
    };

    explicit FriendRequestDialog(QWidget *parent = nullptr, Mode mode = Mode::Apply);
    ~FriendRequestDialog();

    void setSearchInfo(std::shared_ptr<UserProfile> profile);
    void setApplyInfo(std::shared_ptr<PendingFriendApplyRow> info);

    void initTipLbs();
    void addTipLbs(AnimatedStateWidget *lb, QPoint cur_point, QPoint &next_point, int text_width,
                   int text_height);
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void slot_more_lb_clicked();
    void slot_lb_ed_return_pressed();
    void slot_remove_friend_label(QString name);
    void slot_change_friend_label_by_tip(QString lbtext, ClickLabelState state);
    void slot_lb_ed_text_changed(const QString &text);
    void slot_lb_ed_editing_finished();
    void slot_add_friend_label_by_click_tip(QString text);
    void slot_sure_btn_clicked();
    void slot_cancel_btn_clicked();

private:
    void resetLabels();
    void addLabel(QString name);
    void setupUIForMode();

    Ui::FriendRequestDialog *ui;

    Mode _mode;

    QMap<QString, AnimatedStateWidget *> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;

    QMap<QString, FriendLabel *> _friend_labels;
    std::vector<QString> _friend_label_keys;

    std::vector<QString> _tip_data;
    QPoint _tip_cur_point;

    std::shared_ptr<UserProfile> _profile;
    std::shared_ptr<PendingFriendApplyRow> _apply_row;
};

#endif // FRIENDREQUESTDIALOG_H
