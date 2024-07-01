#pragma once

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "../windows/WindowsDirectoryWatcher.h"
#include "../DirectoryWatcher_p.h"
#include <QObject>
#include <QtClassHelperMacros>

class WindowsDirectoryWatcherPrivate final : public DirectoryWatcherPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(WindowsDirectoryWatcher)

  public:
    explicit WindowsDirectoryWatcherPrivate(DirectoryWatcher *qq);
    ~WindowsDirectoryWatcherPrivate() override = default;
    DEFAULT_COPY_MOVE_ROUTINES(WindowsDirectoryWatcherPrivate);

    void setWatchPath(QString path) override;

  private:
    static HANDLE requestDirectoryHandle(QString const &path);

  public Q_SLOTS:
    void dispatchNotify(LPBYTE ptr);

  private:
    QString oldFileName;
};
