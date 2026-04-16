#pragma once
#include "Singleton.h" // 包含你的模板头文件
#include <QJsonObject>
#include <QString>

class ConfigMgr : public Singleton<ConfigMgr>
{
    friend class Singleton<ConfigMgr>;

public:
    QString GetUrlPrefix() const;
private:
    ConfigMgr();
    ~ConfigMgr() override;
    void loadConfig();
    void loadGateServer(const QJsonObject& root);
    
    QString _url_prefix;
};