#include "ChatDialog.h"
#include "ChatUserList.h"
#include "ChatUserWidget.h"
#include "QAction"
#include "global.h"
#include "ui_ChatDialog.h"
#include <QRandomGenerator>
#include <qaction.h>
#include <qicon.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qstringliteral.h>
#include <unistd.h>
#include "LoadingDialog.h"

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
    connect(ui->chat_user_list,&ChatUserList::sig_loading_chat_user,this,&ChatDialog::slot_loading_chat_user);
    addChatUserList();
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