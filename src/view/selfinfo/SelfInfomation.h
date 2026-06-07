#ifndef SELFINFOMATION_H
#define SELFINFOMATION_H

#include <QDialog>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class SelfInfomation; }
QT_END_NAMESPACE

class SelfInfomation : public QDialog
{
    Q_OBJECT

public:
    explicit SelfInfomation(QWidget *parent = nullptr);
    ~SelfInfomation() override;

signals:
    void sig_switch_login();
    void sig_back_chat();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void on_quit_login_btn_clicked();
    void on_back_chat_btn_clicked();

private:
    Ui::SelfInfomation *ui;
    QPoint _drag_pos;
    bool _dragging;
};

#endif // SELFINFOMATION_H
