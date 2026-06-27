#include "ConfigMgr.h"
#include "Log.h"
#include "LogModule.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QString>

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
    LOGI(LogModule::Config, "loading config from {}", config_path.toStdString());

    QFile file(config_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        LOGE(LogModule::Config, "failed to open {}", config_path.toStdString());
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject())
    {
        QJsonObject root = doc.object();
        loadLog(root);
        loadGateServer(root);
        LOGI(LogModule::Config, "config loaded from {}", config_path.toStdString());
    }
    else
    {
        LOGE(LogModule::Config, "config root is not an object {}", config_path.toStdString());
    }
    file.close();
}

namespace
{
spdlog::level::level_enum parseLogLevel(const QString &level_str)
{
    const QString level = level_str.trimmed().toLower();
    if (level == QLatin1String("trace"))
    {
        return spdlog::level::trace;
    }
    if (level == QLatin1String("debug"))
    {
        return spdlog::level::debug;
    }
    if (level == QLatin1String("info"))
    {
        return spdlog::level::info;
    }
    if (level == QLatin1String("warn") || level == QLatin1String("warning"))
    {
        return spdlog::level::warn;
    }
    if (level == QLatin1String("error") || level == QLatin1String("err"))
    {
        return spdlog::level::err;
    }
    if (level == QLatin1String("critical") || level == QLatin1String("fatal"))
    {
        return spdlog::level::critical;
    }
    if (level == QLatin1String("off"))
    {
        return spdlog::level::off;
    }
    return spdlog::level::info;
}
} // namespace

void ConfigMgr::loadLog(const QJsonObject &root)
{
    if (!root.contains("Log") || !root["Log"].isObject())
    {
        LOGW(LogModule::Config, "Log section not found in config");
        return;
    }

    const QJsonObject log = root["Log"].toObject();
    const QString dir = log["Dir"].toString();
    if (!dir.isEmpty())
    {
        _log_config._dir = dir.toStdString();
    }
    const QString level = log["Level"].toString();
    if (!level.isEmpty())
    {
        _log_config._level = parseLogLevel(level);
        LOGI(LogModule::Config, "log level set to {}", level.toStdString());
    }
    else
    {
        LOGW(LogModule::Config, "Log.Level not set, using default 'info'");
    }
}

LogConfig ConfigMgr::getLogConfig() const
{
    return _log_config;
}

void ConfigMgr::loadGateServer(const QJsonObject &root)
{
    if (root.contains("GateServer") && root["GateServer"].isObject())
    {
        QJsonObject gate = root["GateServer"].toObject();
        QString host = gate["host"].toString();
        int port = gate["port"].toInt();

        _url_prefix = QString("http://%1:%2").arg(host).arg(port);
        LOGI(LogModule::Config, "GateServer url={}", _url_prefix.toStdString());
    }
    else
    {
        LOGW(LogModule::Config, "GateServer section not found in config");
    }
}

QString ConfigMgr::getUrlPrefix() const
{
    return _url_prefix;
}
