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

class SearchInfo;

class StatusDialog : public QDialog
{
    Q_OBJECT

public:
    enum class StatusMode
    {
        Fail,    // 纯提示失败（原 FindFailDialog）
        Success, // 展示成功信息可操作（原 FindSuccessDialog）
        Loading  // 加载中动画（原 LoadingDialog）
    };

    explicit StatusDialog(QWidget *parent = nullptr);
    ~StatusDialog() override;

    // 设置模式，切换显示不同 UI 元素
    void setMode(StatusMode mode);

    // 成功模式：设置搜索到的用户信息
    void setSearchInfo(std::shared_ptr<SearchInfo> si);

    // 加载模式：开始/停止动画
    void startLoading();
    void stopLoading();

    // 设置提示文本（用于 fail 模式自定义消息）
    void setTipText(const QString &line1, const QString &line2 = "");

signals:
    // 成功模式点击"添加好友"
    void sig_add_friend(std::shared_ptr<SearchInfo> si);
    // fail 模式点击"确定"
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
    std::shared_ptr<SearchInfo> _si;
};

#endif // STATUSDIALOG_H
