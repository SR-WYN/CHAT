#include "StatusDialog.h"
#include "ui_StatusDialog.h"
#include "UserData.h"
#include <QMovie>
#include <QDebug>

StatusDialog::StatusDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::StatusDialog), _mode(StatusMode::Fail)
{
    ui->setupUi(this);
    setWindowTitle("添加");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setModal(true);
}

StatusDialog::~StatusDialog()
{
    qDebug() << "StatusDialog::~StatusDialog";
    delete ui;
}

void StatusDialog::setMode(StatusMode mode)
{
    _mode = mode;
    switch (mode)
    {
    case StatusMode::Fail:
        showFailUI();
        break;
    case StatusMode::Success:
        showSuccessUI();
        break;
    case StatusMode::Loading:
        showLoadingUI();
        break;
    }
}

void StatusDialog::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    ui->name_label->setText(si->getName());
}

void StatusDialog::startLoading()
{
    if (_mode != StatusMode::Loading)
        return;
    QMovie *movie = new QMovie(":/res/loading.gif");
    ui->loading_label->setMovie(movie);
    movie->start();
    qDebug() << "start loading movie";
}

void StatusDialog::stopLoading()
{
    if (auto *movie = ui->loading_label->movie())
    {
        movie->stop();
    }
}

void StatusDialog::setTipText(const QString &line1, const QString &line2)
{
    if (!line1.isEmpty())
        ui->fail_tip->setText(line1);
    if (!line2.isEmpty())
        ui->fail_tip_2->setText(line2);
}

void StatusDialog::showFailUI()
{
    ui->loading_label->hide();
    ui->success_widget->hide();
    ui->fail_widget->show();
    ui->fail_tip->setText("无法找到该用户");
    ui->fail_tip_2->setText("请检查你填写的账号是否正确");
}

void StatusDialog::showSuccessUI()
{
    ui->loading_label->hide();
    ui->fail_widget->hide();
    ui->success_widget->show();
    // 设置默认头像
    QPixmap head_pix(":/res/head_5.png");
    head_pix = head_pix.scaled(ui->head_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_label->setPixmap(head_pix);
}

void StatusDialog::showLoadingUI()
{
    ui->fail_widget->hide();
    ui->success_widget->hide();
    ui->loading_label->show();
    startLoading();
}

void StatusDialog::on_sure_btn_clicked()
{
    emit sig_sure();
    this->hide();
}

void StatusDialog::on_add_friend_btn_clicked()
{
    if (_si)
    {
        emit sig_add_friend(_si);
    }
    this->hide();
}
