#pragma once

#include <array>
#include <cstddef>
#include <string_view>

enum class LogModule
{
    App,
    Config,
    Tcp,
    Http,
    User,
    Ui,
};

namespace LogNames
{
inline constexpr std::string_view _app = "app";
inline constexpr std::string_view _config = "config";
inline constexpr std::string_view _tcp = "tcp";
inline constexpr std::string_view _http = "http";
inline constexpr std::string_view _user = "user";
inline constexpr std::string_view _ui = "ui";

inline constexpr std::array<std::string_view, 6> _table = {
    _app, _config, _tcp, _http, _user, _ui,
};
} // namespace LogNames

inline std::string_view moduleName(LogModule module)
{
    return LogNames::_table[static_cast<std::size_t>(module)];
}
