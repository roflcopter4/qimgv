#pragma once

#include <QString>
#include <QStringView>
#include <filesystem>
#include <cstddef>

namespace util {

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

template <typename First, typename ...Rest>
void DeleteAndAssignNull(First *&first, Rest *&...rest)
{
    DeleteAndAssignNull(first);
    DeleteAndAssignNull(rest...);
}

QString GetBacktrace();
QString GetErrorMessage(unsigned errVal);

#ifdef Q_OS_WIN32
extern void OpenConsoleWindow();
extern void CloseConsoleWindow();
extern void WaitForAnyKey();
#endif


} // namespace util
