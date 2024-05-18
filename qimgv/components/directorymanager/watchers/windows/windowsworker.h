#pragma once
#ifndef WINDOWSWATCHERWORKER_H
#define WINDOWSWATCHERWORKER_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "../watcherworker.h"
#include <QDebug>
#include <QObject>
#include <QtClassHelperMacros>

class WindowsWorker : public WatcherWorker
{
    Q_OBJECT

  public:
    WindowsWorker() = default;

    void setDirectoryHandle(HANDLE handle);
    void run() override;

  signals:
    void notifyEvent(PFILE_NOTIFY_INFORMATION);

  private:
    static constexpr DWORD POLL_RATE_MS = 1000;

    HANDLE hDir          = INVALID_HANDLE_VALUE;
    DWORD  bytesReturned = 0;

    void freeHandle();
};

#endif // WINDOWSWATCHERWORKER_H
