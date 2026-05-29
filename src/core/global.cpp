#include "global.h"
#include <functional>

std::function<void(QWidget *)> repolish = [](QWidget *w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input)
{
    // 1. 添加“盐值”(Salt)，防止彩虹表攻击
    QString salt = "hello_world";
    QString saltedPassword = input + salt;

    // 2. 将 QString 转为 QByteArray 并进行哈希
    QByteArray data = saltedPassword.toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);

    // 3. 转回十六进制字符串返回
    return QString(hash.toHex());
};
