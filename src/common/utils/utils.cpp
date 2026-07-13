// utils.cpp - 通用工具集合实现
#include "utils.h"

namespace utils::log
{

spdlog::level::level_enum parseLevel(const QString &level_str)
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

} // namespace utils::log
