#include "ApplyFriend.h"
#include "ClickedOnceLabel.h"
#include "CustomsizeEdit.h"
#include "ui_ApplyFriend.h"
#include "UserMgr.h"
#include <QDebug>
#include <QFontMetrics>
#include <QLineEdit>
#include <QScrollBar>
#include <algorithm>

ApplyFriend::ApplyFriend(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplyFriend), _label_point(2, 6)
{
    ui->setupUi(this);
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriend");
    this->setModal(true);
    ui->name_edit->setPlaceholderText(tr("恋恋风辰"));
    ui->label_edit->setPlaceholderText("搜索、添加标签");
    ui->alias_edit->setPlaceholderText("燃烧的胸毛");

    ui->label_edit->setMaxLength(21);
    ui->label_edit->move(2, 2);
    ui->label_edit->setFixedHeight(20);
    ui->label_edit->setMaxLength(10);
    ui->input_tip_widget->hide();

    _tip_cur_point = QPoint(5, 5);

    _tip_data = { "同学","家人","菜鸟教程","C++ Primer","Rust 程序设计",
                             "父与子学Python","nodejs开发指南","go 语言开发指南",
                                "游戏伙伴","金融投资","微信读书","拼多多拼友" };

    connect(ui->more_label, &ClickedOnceLabel::clicked, this, &ApplyFriend::slot_more_lb_clicked);
    initTipLbs();
    //链接输入标签回车事件
    connect(ui->label_edit, &QLineEdit::returnPressed, this, &ApplyFriend::slot_lb_ed_return_pressed);
    connect(ui->label_edit, &QLineEdit::textChanged, this, &ApplyFriend::slot_lb_ed_text_changed);
    connect(ui->label_edit, &QLineEdit::editingFinished, this,
            &ApplyFriend::slot_lb_ed_editing_finished);
    connect(ui->tip_label, &ClickedOnceLabel::clicked, this,
            &ApplyFriend::slot_add_friend_label_by_click_tip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sure_btn->setState("normal", "hover", "press");
    ui->cancel_btn->setState("normal", "hover", "press");
    //连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriend::slot_cancel_btn_clicked);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriend::slot_sure_btn_clicked);
}

ApplyFriend::~ApplyFriend()
{
    qDebug()<< "ApplyFriend destruct";
    delete ui;
}

void ApplyFriend::initTipLbs()
{
    int lines = 1;
    for(int i = 0; i < _tip_data.size(); i++){

        auto* lb = new ClickedLabel(ui->label_list);
        lb->setState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::sig_label_clicked, this, [this, lb]() {
            slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
        });

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textWidth + TIP_OFFSET > ui->label_list->width()) {
            lines++;
            if (lines > 2) {
                delete lb;
                return;
            }

            _tip_cur_point.setX(TIP_OFFSET);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

       auto next_point = _tip_cur_point;

        addTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

       _tip_cur_point = next_point;
    }

}

void ApplyFriend::addTipLbs(ClickedLabel *lb, QPoint cur_point, QPoint &next_point, int text_width,
                            int text_height)
{
    lb->move(cur_point);
    lb->show();
    _add_labels.insert(lb->text(), lb);
    _add_label_keys.push_back(lb->text());
    next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}

bool ApplyFriend::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

void ApplyFriend::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    auto apply_name = UserMgr::getInstancePtr()->getName();
    auto bak_name = si->getName();
    ui->name_edit->setText(apply_name);
    ui->alias_edit->setText(bak_name);
}

void ApplyFriend::slot_more_lb_clicked()
{
    qDebug()<< "receive more label clicked";
    ui->more_label_widget->hide();

    ui->label_list->setFixedWidth(325);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth;
    int textHeight;
    //重拍现有的label
    for(auto & added_key : _add_label_keys){
        auto added_lb = _add_labels[added_key];

        QFontMetrics fontMetrics(added_lb->font()); // 获取QLabel控件的字体信息
        textWidth = fontMetrics.horizontalAdvance(added_lb->text()); // 获取文本的宽度
        textHeight = fontMetrics.height(); // 获取文本的高度

        if(_tip_cur_point.x() +textWidth + TIP_OFFSET > ui->label_list->width()){
            _tip_cur_point.setX(TIP_OFFSET);
            _tip_cur_point.setY(_tip_cur_point.y()+textHeight+15);
        }
        added_lb->move(_tip_cur_point);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());

        _tip_cur_point = next_point;

    }

    //添加未添加的
    for(int i = 0; i < _tip_data.size(); i++){
        auto iter = _add_labels.find(_tip_data[i]);
        if(iter != _add_labels.end()){
            continue;
        }

        auto* lb = new ClickedLabel(ui->label_list);
        lb->setState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::sig_label_clicked, this, [this, lb]() {
            slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
        });

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textWidth + TIP_OFFSET > ui->label_list->width()) {

            _tip_cur_point.setX(TIP_OFFSET);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

         next_point = _tip_cur_point;

        addTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

        _tip_cur_point = next_point;

    }

   int diff_height = next_point.y() + textHeight + TIP_OFFSET - ui->label_list->height();
   ui->label_list->setFixedHeight(next_point.y() + textHeight + TIP_OFFSET);

    //qDebug()<<"after resize ui->label_list size is " <<  ui->label_list->size();
    ui->scroll_content->setFixedHeight(ui->scroll_content->height()+diff_height);
}

void ApplyFriend::resetLabels()
{
    auto max_width = ui->grid_widget->width();
    auto label_height = 0;
    for(auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++){
        //todo... 添加宽度统计
        if( _label_point.x() + iter.value()->width() > max_width) {
            _label_point.setY(_label_point.y()+iter.value()->height()+6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x()+iter.value()->width()+2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }

    if(_friend_labels.isEmpty()){
         ui->label_edit->move(_label_point);
         return;
    }

    if(_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width()){
        ui->label_edit->move(2,_label_point.y()+label_height+6);
    }else{
         ui->label_edit->move(_label_point);
    }
}

void ApplyFriend::addLabel(QString name)
{
    if (_friend_labels.find(name) != _friend_labels.end()) {
        return;
    }

    auto *tmp_label = new FriendLabel(ui->grid_widget);
    tmp_label->setText(name);
    tmp_label->setObjectName("FriendLabel");

    auto max_width = ui->grid_widget->width();
    //todo... 添加宽度统计
    if (_label_point.x() + tmp_label->width() > max_width) {
        _label_point.setY(_label_point.y() + tmp_label->height() + 6);
        _label_point.setX(2);
    }
    else {

    }


    tmp_label->move(_label_point);
    tmp_label->show();
    _friend_labels[tmp_label->text()] = tmp_label;
    _friend_label_keys.push_back(tmp_label->text());

    connect(tmp_label, &FriendLabel::sig_close, this, &ApplyFriend::slot_remove_friend_label);

    _label_point.setX(_label_point.x() + tmp_label->width() + 2);

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width()) {
        ui->label_edit->move(2, _label_point.y() + tmp_label->height() + 2);
    }
    else {
        ui->label_edit->move(_label_point);
    }

    ui->label_edit->clear();

    if (ui->grid_widget->height() < _label_point.y() + tmp_label->height() + 2) {
        ui->grid_widget->setFixedHeight(_label_point.y() + tmp_label->height() * 2 + 2);
    }
}

void ApplyFriend::slot_lb_ed_return_pressed()
{
    if(ui->label_edit->text().isEmpty()){
        return;
    }

    auto text = ui->label_edit->text();
    addLabel(ui->label_edit->text());

    ui->input_tip_widget->hide();
    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->setCurState(ClickLabelState::SELECTED);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->label_list);
    lb->setState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::sig_label_clicked, this, [this, lb]() {
        slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
    });
    qDebug() << "ui->label_list->width() is " << ui->label_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth + TIP_OFFSET + 3 > ui->label_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    addTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + TIP_OFFSET - ui->label_list->height();
    ui->label_list->setFixedHeight(next_point.y() + textHeight + TIP_OFFSET);

    lb->setCurState(ClickLabelState::SELECTED);

    ui->scroll_content->setFixedHeight(ui->scroll_content->height() + diff_height);
}

void ApplyFriend::slot_remove_friend_label(QString name)
{
    qDebug() << "receive close signal";

    _label_point.setX(2);
    _label_point.setY(6);

   auto find_iter = _friend_labels.find(name);

   if(find_iter == _friend_labels.end()){
       return;
   }

   auto find_key = _friend_label_keys.end();
   for(auto iter = _friend_label_keys.begin(); iter != _friend_label_keys.end();
       iter++){
       if(*iter == name){
           find_key = iter;
           break;
       }
   }

   if(find_key != _friend_label_keys.end()){
      _friend_label_keys.erase(find_key);
   }


   delete find_iter.value();

   _friend_labels.erase(find_iter);

   resetLabels();

   auto find_add = _add_labels.find(name);
   if(find_add == _add_labels.end()){
        return;
   }

    find_add.value()->resetNormalState();
}

//点击标已有签添加或删除新联系人的标签
void ApplyFriend::slot_change_friend_label_by_tip(QString lbtext, ClickLabelState state)
{
    auto find_iter = _add_labels.find(lbtext);
    if (find_iter == _add_labels.end()) {
        return;
    }

    if (state == ClickLabelState::SELECTED) {
        //编写添加逻辑
        addLabel(lbtext);
        return;
    }

    if (state == ClickLabelState::NORMAL) {
        //编写删除逻辑
        slot_remove_friend_label(lbtext);
        return;
    }
}

void ApplyFriend::slot_lb_ed_text_changed(const QString &text)
{
    if (text.isEmpty()) {
        ui->tip_label->setText("");
        ui->input_tip_widget->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        auto new_text = ADD_PREFIX + text;
        ui->tip_label->setText(new_text);
        ui->input_tip_widget->show();
        return;
    }
    ui->tip_label->setText(text);
    ui->input_tip_widget->show();
}

void ApplyFriend::slot_lb_ed_editing_finished()
{
    ui->input_tip_widget->hide();
}

void ApplyFriend::slot_add_friend_label_by_click_tip(QString text)
{
    int index = text.indexOf(ADD_PREFIX);
    if (index != -1) {
        text = text.mid(index + ADD_PREFIX.length());
    }
    addLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->setCurState(ClickLabelState::SELECTED);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->label_list);
    lb->setState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::sig_label_clicked, this, [this, lb]() {
        slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
    });
    qDebug() << "ui->label_list->width() is " << ui->label_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth+ TIP_OFFSET+3 > ui->label_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    addTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + TIP_OFFSET - ui->label_list->height();
    ui->label_list->setFixedHeight(next_point.y() + textHeight + TIP_OFFSET);

    lb->setCurState(ClickLabelState::SELECTED);

    ui->scroll_content->setFixedHeight(ui->scroll_content->height()+ diff_height );
}

void ApplyFriend::slot_cancel_btn_clicked()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}

void ApplyFriend::slot_sure_btn_clicked()
{
    qDebug()<<"Slot Apply Sure called" ;
    this->hide();
    deleteLater();
}