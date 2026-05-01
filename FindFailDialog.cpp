#include "FindFailDialog.h"
#include "ui_FindFailDialog.h"
#include <qdebug.h>

FindFailDialog::FindFailDialog(QWidget *parent) : QDialog(parent), ui(new Ui::FindFailDialog)
{
    ui->setupUi(this);
    setWindowTitle("添加");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FindFailDialog");
    this->setModal(true);
}

FindFailDialog::~FindFailDialog()
{
    qDebug() << "FindFailDialog::~FindFailDialog";
    delete ui;
}

void FindFailDialog::on_fail_sure_btn_clicked()
{
    this->hide();
}
