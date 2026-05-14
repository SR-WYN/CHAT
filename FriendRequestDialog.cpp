#include "FriendRequestDialog.h"
#include "AnimatedStateWidget.h"
#include "CustomsizeEdit.h"
#include "FriendLabel.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "ui_FriendRequestDialog.h"
#include <QDebug>
#include <QFontMetrics>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMouseEvent>
#include <QScrollBar>
#include <algorithm>

FriendRequestDialog::FriendRequestDialog(QWidget *parent, Mode mode)
    : QDialog(parent), ui(new Ui::FriendRequestDialog), _mode(mode), _label_point(2, 6)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FriendRequestDialog");
    this->setModal(true);

    setupUIForMode();

    ui->label_edit->setMaxLength(21);
    ui->label_edit->move(2, 2);
    ui->label_edit->setFixedHeight(35);
    ui->label_edit->setMaxLength(10);
    ui->input_tip_widget->hide();

    _tip_cur_point = QPoint(5, 5);

    _tip_data = {"同学",          "家人",           "菜鸟教程",       "C++ Primer",
                 "Rust 程序设计", "父与子学Python", "nodejs开发指南", "go 语言开发指南",
                 "游戏伙伴",      "金融投资",       "微信读书",       "拼多多拼友"};

    ui->tip_label->installEventFilter(this);
    ui->more_label->installEventFilter(this);
    initTipLbs();
    connect(ui->label_edit, &QLineEdit::returnPressed, this,
            &FriendRequestDialog::slot_lb_ed_return_pressed);
    connect(ui->label_edit, &QLineEdit::textChanged, this,
            &FriendRequestDialog::slot_lb_ed_text_changed);
    connect(ui->label_edit, &QLineEdit::editingFinished, this,
            &FriendRequestDialog::slot_lb_ed_editing_finished);
    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    connect(ui->cancel_btn, &QPushButton::clicked, this, &FriendRequestDialog::slot_cancel_btn_clicked);
    connect(ui->sure_btn, &QPushButton::clicked, this, &FriendRequestDialog::slot_sure_btn_clicked);
}

FriendRequestDialog::~FriendRequestDialog()
{
    qDebug() << "FriendRequestDialog destruct";
    delete ui;
}

void FriendRequestDialog::setupUIForMode()
{
    if (_mode == Mode::Apply)
    {
        setWindowTitle("添加");
        ui->name_edit->setPlaceholderText(tr("我的名称"));
        ui->label_edit->setPlaceholderText("搜索、添加标签");
        ui->alias_edit->setPlaceholderText(tr("备注名"));
        ui->apply_label->setText("发送添加好友申请：");
    }
    else
    {
        setWindowTitle("认证");
        ui->name_edit->setPlaceholderText(tr("对方看到的我的名称"));
        ui->label_edit->setPlaceholderText(tr("为好友添加标签"));
        ui->alias_edit->setPlaceholderText(tr("给好友的备注"));
        ui->apply_label->setText("确认好友申请：");
    }
}

void FriendRequestDialog::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    auto apply_name = UserMgr::getInstancePtr()->getName();
    auto bak_name = si->getName();
    ui->name_edit->setText(apply_name);
    ui->alias_edit->setText(bak_name);
}

void FriendRequestDialog::setApplyInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info = apply_info;
    if (!apply_info)
        return;

    const QString my_name = UserMgr::getInstance().getName();
    ui->name_edit->setText(my_name);

    const QString peer_display =
        apply_info->_nick.isEmpty() ? apply_info->_name : apply_info->_nick;
    ui->alias_edit->setText(peer_display);
    ui->alias_edit->setPlaceholderText(peer_display);
}

// ---------- 标签系统 ----------

void FriendRequestDialog::initTipLbs()
{
    int lines = 1;
    for (int i = 0; i < _tip_data.size(); i++)
    {
        auto *lb = new AnimatedStateWidget(ui->label_list);
        lb->setQssInteraction(AnimatedStateWidget::QssInteraction::ToggleSelection);
        lb->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                     "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &AnimatedStateWidget::clicked, this, [this, lb]() {
            slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
        });

        QFontMetrics fontMetrics(lb->font());
        int textWidth = fontMetrics.horizontalAdvance(lb->text());
        int textHeight = fontMetrics.height();

        if (_tip_cur_point.x() + textWidth + TIP_OFFSET > ui->label_list->width())
        {
            lines++;
            if (lines > 2)
            {
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

void FriendRequestDialog::addTipLbs(AnimatedStateWidget *lb, QPoint cur_point, QPoint &next_point,
                                    int text_width, int text_height)
{
    lb->move(cur_point);
    lb->show();
    _add_labels.insert(lb->text(), lb);
    _add_label_keys.push_back(lb->text());
    next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}

bool FriendRequestDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton)
        {
            if (obj == ui->tip_label)
            {
                slot_add_friend_label_by_click_tip(ui->tip_label->text());
                return true;
            }
            if (obj == ui->more_label)
            {
                slot_more_lb_clicked();
                return true;
            }
        }
    }
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

void FriendRequestDialog::slot_more_lb_clicked()
{
    qDebug() << "receive more label clicked";
    ui->more_label_widget->hide();

    ui->label_list->setFixedWidth(325);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth;
    int textHeight;
    // 重拍现有的label
    for (auto &added_key : _add_label_keys)
    {
        auto added_lb = _add_labels[added_key];

        QFontMetrics fontMetrics(added_lb->font());
        textWidth = fontMetrics.horizontalAdvance(added_lb->text());
        textHeight = fontMetrics.height();

        if (_tip_cur_point.x() + textWidth + TIP_OFFSET > ui->label_list->width())
        {
            _tip_cur_point.setX(TIP_OFFSET);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }
        added_lb->move(_tip_cur_point);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());

        _tip_cur_point = next_point;
    }

    // 添加未添加的
    for (int i = 0; i < _tip_data.size(); i++)
    {
        auto iter = _add_labels.find(_tip_data[i]);
        if (iter != _add_labels.end())
        {
            continue;
        }

        auto *lb = new AnimatedStateWidget(ui->label_list);
        lb->setQssInteraction(AnimatedStateWidget::QssInteraction::ToggleSelection);
        lb->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                     "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &AnimatedStateWidget::clicked, this, [this, lb]() {
            slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
        });

        QFontMetrics fontMetrics(lb->font());
        textWidth = fontMetrics.horizontalAdvance(lb->text());
        textHeight = fontMetrics.height();

        if (_tip_cur_point.x() + textWidth + TIP_OFFSET > ui->label_list->width())
        {
            _tip_cur_point.setX(TIP_OFFSET);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }

        next_point = _tip_cur_point;

        addTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

        _tip_cur_point = next_point;
    }

    int diff_height = next_point.y() + textHeight + TIP_OFFSET - ui->label_list->height();
    ui->label_list->setFixedHeight(next_point.y() + textHeight + TIP_OFFSET);

    ui->scroll_content->setFixedHeight(ui->scroll_content->height() + diff_height);
}

void FriendRequestDialog::resetLabels()
{
    auto max_width = ui->grid_widget->width();
    auto label_height = 0;
    for (auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++)
    {
        if (_label_point.x() + iter.value()->width() > max_width)
        {
            _label_point.setY(_label_point.y() + iter.value()->height() + 6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x() + iter.value()->width() + 2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }

    if (_friend_labels.isEmpty())
    {
        ui->label_edit->move(_label_point);
        return;
    }

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width())
    {
        ui->label_edit->move(2, _label_point.y() + label_height + 6);
    }
    else
    {
        ui->label_edit->move(_label_point);
    }
}

void FriendRequestDialog::addLabel(QString name)
{
    if (_friend_labels.find(name) != _friend_labels.end())
    {
        return;
    }

    auto *tmp_label = new FriendLabel(ui->grid_widget);
    tmp_label->setText(name);
    tmp_label->setObjectName("FriendLabel");

    auto max_width = ui->grid_widget->width();
    if (_label_point.x() + tmp_label->width() > max_width)
    {
        _label_point.setY(_label_point.y() + tmp_label->height() + 6);
        _label_point.setX(2);
    }

    tmp_label->move(_label_point);
    tmp_label->show();
    _friend_labels[tmp_label->text()] = tmp_label;
    _friend_label_keys.push_back(tmp_label->text());

    connect(tmp_label, &FriendLabel::sig_close, this, &FriendRequestDialog::slot_remove_friend_label);

    _label_point.setX(_label_point.x() + tmp_label->width() + 2);

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width())
    {
        ui->label_edit->move(2, _label_point.y() + tmp_label->height() + 2);
    }
    else
    {
        ui->label_edit->move(_label_point);
    }

    ui->label_edit->clear();

    if (ui->grid_widget->height() < _label_point.y() + tmp_label->height() + 2)
    {
        ui->grid_widget->setFixedHeight(_label_point.y() + tmp_label->height() * 2 + 2);
    }
}

void FriendRequestDialog::slot_lb_ed_return_pressed()
{
    if (ui->label_edit->text().isEmpty())
    {
        return;
    }

    auto text = ui->label_edit->text();
    addLabel(ui->label_edit->text());

    ui->input_tip_widget->hide();
    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (find_it == _tip_data.end())
    {
        _tip_data.push_back(text);
    }

    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end())
    {
        find_add.value()->setCurState(ClickLabelState::SELECTED);
        return;
    }

    auto *lb = new AnimatedStateWidget(ui->label_list);
    lb->setQssInteraction(AnimatedStateWidget::QssInteraction::ToggleSelection);
    lb->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                 "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &AnimatedStateWidget::clicked, this, [this, lb]() {
        slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
    });
    qDebug() << "ui->label_list->width() is " << ui->label_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font());
    int textWidth = fontMetrics.horizontalAdvance(lb->text());
    int textHeight = fontMetrics.height();
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth + TIP_OFFSET + 3 > ui->label_list->width())
    {
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

void FriendRequestDialog::slot_remove_friend_label(QString name)
{
    qDebug() << "receive close signal";

    _label_point.setX(2);
    _label_point.setY(6);

    auto find_iter = _friend_labels.find(name);

    if (find_iter == _friend_labels.end())
    {
        return;
    }

    auto find_key = _friend_label_keys.end();
    for (auto iter = _friend_label_keys.begin(); iter != _friend_label_keys.end(); iter++)
    {
        if (*iter == name)
        {
            find_key = iter;
            break;
        }
    }

    if (find_key != _friend_label_keys.end())
    {
        _friend_label_keys.erase(find_key);
    }

    delete find_iter.value();

    _friend_labels.erase(find_iter);

    resetLabels();

    auto find_add = _add_labels.find(name);
    if (find_add == _add_labels.end())
    {
        return;
    }

    find_add.value()->resetNormalState();
}

void FriendRequestDialog::slot_change_friend_label_by_tip(QString lbtext, ClickLabelState state)
{
    auto find_iter = _add_labels.find(lbtext);
    if (find_iter == _add_labels.end())
    {
        return;
    }

    if (state == ClickLabelState::SELECTED)
    {
        addLabel(lbtext);
        return;
    }

    if (state == ClickLabelState::NORMAL)
    {
        slot_remove_friend_label(lbtext);
        return;
    }
}

void FriendRequestDialog::slot_lb_ed_text_changed(const QString &text)
{
    if (text.isEmpty())
    {
        ui->tip_label->setText("");
        ui->input_tip_widget->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end())
    {
        auto new_text = ADD_PREFIX + text;
        ui->tip_label->setText(new_text);
        ui->input_tip_widget->show();
        return;
    }
    ui->tip_label->setText(text);
    ui->input_tip_widget->show();
}

void FriendRequestDialog::slot_lb_ed_editing_finished()
{
    ui->input_tip_widget->hide();
}

void FriendRequestDialog::slot_add_friend_label_by_click_tip(QString text)
{
    int index = text.indexOf(ADD_PREFIX);
    if (index != -1)
    {
        text = text.mid(index + ADD_PREFIX.length());
    }
    addLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (find_it == _tip_data.end())
    {
        _tip_data.push_back(text);
    }

    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end())
    {
        find_add.value()->setCurState(ClickLabelState::SELECTED);
        return;
    }

    auto *lb = new AnimatedStateWidget(ui->label_list);
    lb->setQssInteraction(AnimatedStateWidget::QssInteraction::ToggleSelection);
    lb->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                 "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &AnimatedStateWidget::clicked, this, [this, lb]() {
        slot_change_friend_label_by_tip(lb->text(), lb->getCurState());
    });
    qDebug() << "ui->label_list->width() is " << ui->label_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font());
    int textWidth = fontMetrics.horizontalAdvance(lb->text());
    int textHeight = fontMetrics.height();
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth + TIP_OFFSET + 3 > ui->label_list->width())
    {
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

void FriendRequestDialog::slot_cancel_btn_clicked()
{
    qDebug() << "cancel";
    reject();
}

void FriendRequestDialog::slot_sure_btn_clicked()
{
    if (_mode == Mode::Apply)
    {
        qDebug() << "Slot Apply Sure called";
        QJsonObject json_obj;
        auto uid = UserMgr::getInstance().getUid();
        json_obj["uid"] = uid;
        auto name = ui->name_edit->text();
        if (name.isEmpty())
        {
            name = ui->name_edit->placeholderText();
        }
        json_obj["apply_name"] = name;
        auto alias_name = ui->alias_edit->text();
        if (alias_name.isEmpty())
        {
            alias_name = ui->alias_edit->placeholderText();
        }
        json_obj["alias_name"] = alias_name;
        json_obj["touid"] = _si->getUid();
        QJsonDocument doc(json_obj);
        QByteArray json_data = doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::getInstance().sig_send_data(ReqId::ID_ADD_FRIEND_REQ, json_data);
    }
    else
    {
        qDebug() << "AuthenFriend: confirm auth (accept apply)";
        if (!_apply_info)
        {
            qDebug() << "FriendRequestDialog: missing apply info";
            return;
        }

        QJsonObject json_obj;
        const int self_uid = UserMgr::getInstance().getUid();
        const int applicant_uid = _apply_info->_uid;

        json_obj["fromuid"] = applicant_uid;
        json_obj["touid"] = self_uid;
        QString alias_name = "";
        if (!ui->alias_edit->text().isEmpty())
        {
            alias_name = ui->alias_edit->text();
        }
        else
        {
            alias_name = ui->alias_edit->placeholderText();
        }
        json_obj["alias_name"] = alias_name;
        QJsonDocument doc(json_obj);
        QByteArray json_data = doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::getInstance().sig_send_data(ReqId::ID_AUTH_FRIEND_REQ, json_data);
    }

    accept();
}
