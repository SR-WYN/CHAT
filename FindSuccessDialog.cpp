#include "FindSuccessDialog.h"
#include "ui_FindSuccessDialog.h"
#include <QDir>
#include <memory>
#include "UserData.h"

FindSuccessDialog::FindSuccessDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::FindSuccessDialog)
{
    ui->setupUi(this);
    // 设置对话框标题
    setWindowTitle("添加");
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    QString pix_path = QDir::toNativeSeparators(app_path + QDir::separator() + "static" +
                                                QDir::separator() + "head_5.png");
    QPixmap head_pix(pix_path);
    head_pix =
        head_pix.scaled(ui->head_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_label->setPixmap(head_pix);
    ui->add_friend_btn->setState("normal", "hover", "press");
    this->setModal(true);
}

FindSuccessDialog::~FindSuccessDialog()
{
    delete ui;
}

void FindSuccessDialog::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    ui->name_label->setText(si->getName());
    _si = si;
}

void FindSuccessDialog::on_add_friend_btn_clicked()
{
    //todo 添加好友
    this->hide();
}