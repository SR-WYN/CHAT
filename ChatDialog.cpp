#include "ChatDialog.h"
#include "ContactUserList.h"
#include "ChatUserList.h"
#include "ChatUserWidget.h"
#include "LoadingDialog.h"
#include "QAction"
#include "global.h"
#include "ui_ChatDialog.h"
#include <QMouseEvent>
#include <QRandomGenerator>
#include <qaction.h>
#include <qicon.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qstringliteral.h>
#include <unistd.h>

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatDialog), _mode(ChatUIMode::CHAT_MODE),
      _state(ChatUIMode::CHAT_MODE), _b_loading(false)
{
    ui->setupUi(this);
    ui->add_btn->setState("normal", "hover", "press");
    ui->search_edit->setMaxLength(15);

    QAction *search_action = new QAction(ui->search_edit);
    search_action->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(search_action, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    QAction *clear_action = new QAction(ui->search_edit);
    clear_action->setIcon(QIcon(":/res/empty.png"));
    ui->search_edit->addAction(clear_action, QLineEdit::TrailingPosition);

    connect(ui->search_edit, &QLineEdit::textChanged, [clear_action](const QString &text) {
        if (text.isEmpty())
        {
            clear_action->setIcon(QIcon(":/res/empty.png"));
        }
        else
        {
            clear_action->setIcon(QIcon(":/res/close_search.png"));
        }
    });

    connect(clear_action, &QAction::triggered, [this, clear_action]() {
        ui->search_edit->clear();
        clear_action->setIcon(QIcon(":/res/empty.png"));
        ui->search_edit->clearFocus();
        showSearch(false);
    });

    showSearch(false);
    // 与侧栏默认「聊天」选中一致，避免 UI currentIndex 仍为好友页导致右侧不对
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this,
            &ChatDialog::slot_loading_chat_user);
    addChatUserList();

    addLabelGroup(ui->side_chat_label);
    addLabelGroup(ui->side_contact_label);
    ui->side_chat_label->setSelected(true);
    ui->side_contact_label->setSelected(false);

    connect(ui->side_chat_label, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_label, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);

    connect(ui->con_user_list, &ContactUserList::sig_switch_apply_friend_page, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    });

    // 链接搜索框输入变化
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);
    ui->search_list->setSearchEdit(ui->search_edit);

    // 安装事件过滤器
    this->installEventFilter(this);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::showSearch(bool bsearch)
{
    if (bsearch)
    {
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SEARCH_MODE;
    }
    else if (_state == ChatUIMode::CHAT_MODE)
    {
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_list->hide();
        _mode = ChatUIMode::CHAT_MODE;
    }
    else if (_state == ChatUIMode::CONTACT_MODE)
    {
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::CONTACT_MODE;
    }
}

std::vector<QString> strs = {"hello world !", "nice to meet u", "New year，new life",
                             "You have to love yourself",
                             "My love is written in the wind ever since the whole world is you"};
std::vector<QString> heads = {":/res/head_1.png", ":/res/head_2.png", ":/res/head_3.png",
                              ":/res/head_4.png", ":/res/head_5.png"};
std::vector<QString> names = {"llfc", "zack", "golang", "cpp", "java", "nodejs", "python", "rust"};

void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    for (int i = 0; i < 13; i++)
    {
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue % strs.size();
        int head_i = randomValue % heads.size();
        int name_i = randomValue % names.size();
        auto *chat_user_wid = new ChatUserWidget();
        chat_user_wid->setInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}

void ChatDialog::slot_loading_chat_user()
{
    if (_b_loading)
    {
        return;
    }
    _b_loading = true;
    LoadingDialog *loading_dialog = new LoadingDialog(this);
    loading_dialog->setModal(true);
    loading_dialog->show();
    qDebug() << "add new data to list...";
    addChatUserList();
    loading_dialog->deleteLater();
    _b_loading = false;
}

void ChatDialog::addLabelGroup(StateWidget *label)
{
    _label_list.push_back(label);
}

void ChatDialog::slot_side_chat()
{
    qDebug() << "receive side chat clicked";
    clearLabelState(ui->side_chat_label);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::CHAT_MODE;
    showSearch(false);
}

void ChatDialog::slot_side_contact()
{
    qDebug() << "receive side contact clicked";
    clearLabelState(ui->side_contact_label);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    _state = ChatUIMode::CONTACT_MODE;
    showSearch(false);
}

void ChatDialog::clearLabelState(StateWidget *label)
{
    for (auto &item : _label_list)
    {
        if (item != label)
        {
            item->clearState();
        }
    }
}

void ChatDialog::slot_text_changed(const QString &str)
{
    if (!str.isEmpty())
    {
        showSearch(true);
    }
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        handleGlobalMousePress(mouse_event);
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *mouse_event)
{
    // 实现点击位置的判断和处理逻辑
    // 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
    if (_mode != ChatUIMode::SEARCH_MODE)
    {
        return;
    }

    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPoint posInSearchList = ui->search_list->mapFromGlobal(mouse_event->globalPosition().toPoint());
    // 判断点击位置是否在聊天列表的范围内
    if (!ui->search_list->rect().contains(posInSearchList))
    {
        // 如果不在聊天列表内，清空输入框
        ui->search_edit->clear();
        showSearch(false);
    }
}