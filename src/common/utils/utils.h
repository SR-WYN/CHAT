// utils.h - 通用工具集合
#pragma once

#include <QLatin1String>
#include <QString>
#include <spdlog/common.h>

namespace utils::log
{

spdlog::level::level_enum parseLevel(const QString &level_str);

} // namespace utils::log
