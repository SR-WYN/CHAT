#pragma once
#include "Log.h"
#include "Singleton.h"
#include <QJsonObject>
#include <QString>

class ConfigMgr : public Singleton<ConfigMgr>
{
    friend class Singleton<ConfigMgr>;

public:
    QString getUrlPrefix() const;
    LogConfig getLogConfig() const;

private:
    ConfigMgr();
    ~ConfigMgr() override;
    void loadConfig();
    void loadGateServer(const QJsonObject &root);
    void loadLog(const QJsonObject &root);

    QString _url_prefix;
    LogConfig _log_config;
};