#include "FriendLabel.h"
#include "ClickedLabel.h"
#include "ui_FriendLabel.h"

FriendLabel::FriendLabel(QWidget *parent) : QFrame(parent), ui(new Ui::FriendLabel)
{
    ui->setupUi(this);
    ui->tip_close->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                            "selected_pressed");
    connect(ui->tip_close, &ClickedLabel::sig_label_clicked, this, &FriendLabel::slot_close);
}

FriendLabel::~FriendLabel()
{
    delete ui;
}

void FriendLabel::setText(QString text)
{
    _text = text;
    ui->tip_label->setText(_text);
    QFontMetrics fontMetrics(ui->tip_label->font());
    auto text_width = fontMetrics.horizontalAdvance(ui->tip_label->text());
    auto text_height = fontMetrics.height();
    constexpr int horizontal_padding = 10;
    constexpr int vertical_padding = 8;
    constexpr int close_btn_width = 15;
    constexpr int close_wrap_width = 25;

    ui->tip_label->setFixedWidth(text_width + horizontal_padding);
    this->setFixedWidth(text_width + close_wrap_width + horizontal_padding);
    this->setFixedHeight(std::max(text_height + vertical_padding, close_btn_width + vertical_padding));
    _width = QWidget::width();
    _height = QWidget::height();
}

int FriendLabel::width()
{
    return _width;
}

int FriendLabel::height()
{
    return _height;
}

QString FriendLabel::text()
{
    return _text;
}

void FriendLabel::slot_close()
{
    emit sig_close(_text);
}
