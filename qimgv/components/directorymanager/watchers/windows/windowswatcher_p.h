#ifndef WINDOWSWATCHER_P_H
#define WINDOWSWATCHER_P_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "../windows/windowswatcher.h"
#include "../directorywatcher_p.h"
#include <QDebug>
#include <QObject>
#include <QtClassHelperMacros>

class WindowsWatcherPrivate final : public DirectoryWatcherPrivate
{
    Q_OBJECT

  public:
    explicit WindowsWatcherPrivate(WindowsWatcher *qq = nullptr);
    ~WindowsWatcherPrivate() override = default;

    static HANDLE requestDirectoryHandle(QString const &path);

    QString oldFileName;

    Q_DISABLE_COPY_MOVE(WindowsWatcherPrivate)

  public slots:
    void dispatchNotify(PFILE_NOTIFY_INFORMATION notify);

  private:
    Q_DECLARE_PUBLIC(WindowsWatcher)
};

#endif // WINDOWSWATCHER_P_H
