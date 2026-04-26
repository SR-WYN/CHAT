#include "LoadingDialog.h"
#include "ui_LoadingDialog.h"
#include <qdebug.h>
#include <qmovie.h>

LoadingDialog::LoadingDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoadingDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
                   Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(parent->size());
    QMovie *movie = new QMovie(":/res/loading.gif");
    ui->loading_label->setMovie(movie);
    movie->start();
    qDebug() << "start loading movie";
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}
