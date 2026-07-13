#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include <QDialog>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class StatusDialog;
}
QT_END_NAMESPACE
namespace Ui
{
class StatusDialog;
}
QT_END_NAMESPACE

struct UserProfile;

class StatusDialog : public QDialog
{
    Q_OBJECT

public:
    enum class StatusMode
    {
        Fail,
        Success,
        Loading
    };

    explicit StatusDialog(QWidget *parent = nullptr);
    ~StatusDialog() override;

    void setMode(StatusMode mode);
    void setSearchInfo(std::shared_ptr<UserProfile> profile);
    void startLoading();
    void stopLoading();
    void setTipText(const QString &line1, const QString &line2 = "");

signals:
    void sig_add_friend(std::shared_ptr<UserProfile> profile);
    void sig_sure();

private slots:
    void on_sure_btn_clicked();
    void on_add_friend_btn_clicked();

private:
    void showFailUI();
    void showSuccessUI();
    void showLoadingUI();

    Ui::StatusDialog *ui;
    StatusMode _mode;
    std::shared_ptr<UserProfile> _profile;
};

#endif // STATUSDIALOG_H
