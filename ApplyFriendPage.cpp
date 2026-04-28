#include "ApplyFriendPage.h"
#include "ApplyFriendItem.h"
#include "ui_ApplyFriendPage.h"
#include "UserData.h"
#include <QListWidgetItem>
#include <QRandomGenerator>

namespace {
void loadMockFriendApplies(class Ui::ApplyFriendPage *ui)
{
    const QStringList strs = {
        QStringLiteral("nice to meet u"),
        QStringLiteral("hello world !"),
        QStringLiteral("My love is written in the wind ever since the whole world is you"),
        QStringLiteral("hello world !"),
        QStringLiteral("nice to meet u"),
    };
    const QStringList heads = {
        QStringLiteral(":/res/head_1.png"),
        QStringLiteral(":/res/head_2.png"),
        QStringLiteral(":/res/head_3.png"),
        QStringLiteral(":/res/head_4.png"),
        QStringLiteral(":/res/head_5.png"),
    };
    const QStringList names = {
        QStringLiteral("Candy"),
        QStringLiteral("HanMeiMei"),
        QStringLiteral("Hunter"),
        QStringLiteral("Max"),
        QStringLiteral("Androw"),
        QStringLiteral("Lily"),
        QStringLiteral("Ben"),
    };

    for (int i = 0; i < 13; ++i)
    {
        const int r = QRandomGenerator::global()->bounded(100);
        auto info =
            std::make_shared<ApplyInfo>(1000 + i, names[r % names.size()], strs[r % strs.size()],
                                        heads[r % heads.size()], names[r % names.size()], 0, 0);
        auto *itemW = new ApplyFriendItem();
        itemW->SetInfo(info);
        itemW->ShowAddBtn(false);

        auto *lwItem = new QListWidgetItem;
        lwItem->setSizeHint(itemW->sizeHint());
        ui->apply_friend_list->addItem(lwItem);
        ui->apply_friend_list->setItemWidget(lwItem, itemW);
    }
}
} // namespace

ApplyFriendPage::ApplyFriendPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApplyFriendPage)
{
    ui->setupUi(this);
    loadMockFriendApplies(ui);
}

ApplyFriendPage::~ApplyFriendPage()
{
    delete ui;
}
