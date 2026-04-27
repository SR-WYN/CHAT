#ifndef FINDSUCCESSDIALOG_H
#define FINDSUCCESSDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FindSuccessDialog;
}
QT_END_NAMESPACE

class SearchInfo;

class FindSuccessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindSuccessDialog(QWidget *parent = nullptr);
    ~FindSuccessDialog() override;
    void setSearchInfo(std::shared_ptr<SearchInfo> si);
private slots:
    void on_add_friend_btn_clicked();
private:
    Ui::FindSuccessDialog *ui;
    QWidget *_parent;
    std::shared_ptr<SearchInfo> _si;
};

#endif // FINDSUCCESSDIALOG_H
