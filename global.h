#pragma once

#include <QWidget>
#include <functional>
#include "QStyle"
#include <QRegularExpression>

//用来刷新qss
extern std::function<void(QWidget*)> repolish;