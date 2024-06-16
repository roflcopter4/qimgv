#pragma once

#include <QString>
#include <QStringView>
#include <filesystem>
#include <cstddef>

#ifdef _WIN32
# include <basetsd.h>
using StdString = std::wstring;
using ssize_t = SSIZE_T;
#else
using StdString = std::string;
#endif

int clamp(int x, int lower, int upper);
int probeOS();

namespace util {

#if defined Q_OS_WIN32 && false
inline constexpr QChar pathsep = u'\\';
#else
inline constexpr QChar pathsep = u'/';
#endif

using namespace Qt::Literals::StringLiterals;

auto StdPathToQString(std::filesystem::path const &filePath) -> QString;
auto QStringToStdPath(QString const &filePath) -> std::filesystem::path;

inline QString QStringViewToQString(QStringView const &view)
{
    return QString{view.data(), view.size()};
}

template <typename Ty>
void DeleteAndAssignNull(Ty *&var)
{
    delete var;
    var = nullptr;
}

QString GetBacktrace();
QString GetErrorMessage(unsigned errVal);

#ifdef Q_OS_WIN32
extern void OpenConsoleWindow();
extern void CloseConsoleWindow();
extern void WaitForAnyKey();
#endif


} // namespace util
