#pragma once
#ifndef WINDOWSWATCHERWORKER_H
#define WINDOWSWATCHERWORKER_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "../DirectoryWatcherWorker.h"
#include <QDebug>
#include <QObject>
#include <QtClassHelperMacros>

class WindowsDirectoryWorker final : public DirectoryWatcherWorker
{
    Q_OBJECT

    using PCBYTE = BYTE const *;
    using PCFILE_NOTIFY_INFORMATION = FILE_NOTIFY_INFORMATION const *;

  public:
    WindowsDirectoryWorker();
    ~WindowsDirectoryWorker() override;
    DELETE_COPY_MOVE_ROUTINES(WindowsDirectoryWorker);

    void setDirectoryHandle(HANDLE handle);
    void run() override;

  private:
    void iterateDirectoryEvents(PCBYTE buffer);

  Q_SIGNALS:
    void notifyEvent(LPBYTE);

  private:
    static constexpr DWORD POLL_RATE_MS = 100;
    static constexpr DWORD BUFFER_SIZE  = 1 << 18;
    //static constexpr DWORD BUFFER_SIZE  = 256;

    volatile HANDLE hDir = INVALID_HANDLE_VALUE;
    CRITICAL_SECTION criticalSection;
};

#endif // WINDOWSWATCHERWORKER_H
