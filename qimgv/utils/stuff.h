#pragma once

#include <QString>
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
static constexpr QChar pathsep = QChar(L'\\');
#else
static constexpr QChar pathsep = QChar(L'/');
#endif

StdString QStringToStdString(QString const &str);
QString   StdStringToQString(StdString const &str);
QString   StdPathToQString(std::filesystem::path const &str);

inline std::filesystem::path QStringToStdPath(QString const &filePath)
{
    auto view = std::basic_string_view{filePath.data_ptr().data(),
                                       static_cast<size_t>(filePath.size())};
    return std::filesystem::path{view};
}

inline QString QStringViewToQString(QStringView const &view)
{
    return QString{view.data(), view.size()};
}

QString get_backtrace();

#ifdef Q_OS_WIN32
extern void OpenConsoleWindow();
extern void CloseConsoleWindow();
#endif

} // namespace util
