#include "ConfigMgr.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>

ConfigMgr::ConfigMgr()
{
    loadConfig();
}

ConfigMgr::~ConfigMgr()
{
}

void ConfigMgr::loadConfig()
{
    QString config_path = QDir(QCoreApplication::applicationDirPath()).filePath("config.json");

    QFile file(config_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Critical: config.json missing";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject())
    {
        QJsonObject root = doc.object();

        loadGateServer(root);
    }
    file.close();
}

void ConfigMgr::loadGateServer(const QJsonObject &root)
{
    if (root.contains("GateServer") && root["GateServer"].isObject())
    {
        QJsonObject gate = root["GateServer"].toObject();
        QString host = gate["host"].toString();
        int port = gate["port"].toInt();

        _url_prefix = QString("http://%1:%2").arg(host).arg(port);
        qDebug() << "Config Loaded: Gate URL Prefix is" << _url_prefix;
    }
}

QString ConfigMgr::getUrlPrefix() const
{
    return _url_prefix;
}