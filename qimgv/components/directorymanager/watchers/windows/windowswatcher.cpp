#include "Common.h"
#include "windowswatcher_p.h"
#include "windowsworker.h"
#include "utils/stuff.h"

static QString win32ErrorString(DWORD error)
{
    wchar_t buffer[1024];

    DWORD res = ::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        error,
        LANG_SYSTEM_DEFAULT,
        buffer,
        std::size(buffer),
        nullptr
    );

    return res == 0 ? QString::number(error)
                    : QSV(__FILE__ "::" QT_STRINGIFY(__LINE__) ": ") +
                      QString::fromWCharArray(buffer, res);
}

WindowsWatcherPrivate::WindowsWatcherPrivate(WindowsWatcher* qq)
    : DirectoryWatcherPrivate(qq, new WindowsWorker())
{
    auto *windowsWorker = reinterpret_cast<WindowsWorker*>(worker.data());
    qRegisterMetaType<PFILE_NOTIFY_INFORMATION>("PFILE_NOTIFY_INFORMATION");

    connect(windowsWorker, SIGNAL(notifyEvent(PFILE_NOTIFY_INFORMATION)),
            this, SLOT(dispatchNotify(PFILE_NOTIFY_INFORMATION)));
}

HANDLE WindowsWatcherPrivate::requestDirectoryHandle(QString const &path)
{
    HANDLE hDirectory;
    std::wstring wpath = path.startsWith(QSV(R"(\\?\)"))
        ? path.toStdWString()
        : util::QStringToStdPath(path).wstring();

    do {
        hDirectory = ::CreateFileW(
            wpath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr
        );

        if (hDirectory == INVALID_HANDLE_VALUE) {
            DWORD err = ::GetLastError();
            if (err == ERROR_SHARING_VIOLATION) {
                qDebug() << u"ERROR_SHARING_VIOLATION waiting for 1 sec";
                QThread::sleep(1);
            } else {
                qDebug() << win32ErrorString(err);
                return INVALID_HANDLE_VALUE;
            }
        }
    } while (hDirectory == INVALID_HANDLE_VALUE);

    return hDirectory;
}

void WindowsWatcherPrivate::dispatchNotify(PFILE_NOTIFY_INFORMATION notify)
{
    Q_Q(WindowsWatcher);

    DWORD   len  = notify->FileNameLength / sizeof(WCHAR);
    QString name = QString::fromWCharArray(notify->FileName, len);

    switch (notify->Action) {
    case FILE_ACTION_ADDED:
        emit q->fileCreated(name);
        break;

    case FILE_ACTION_MODIFIED:
        emit q->fileModified(name);
#if 0
        // ??
        WatcherEvent* event;
        if (findEventIndexByName(name) != -1)
            return;
        event = new WatcherEvent(name, WatcherEvent::MODIFIED);
        event->mTimerId = startTimer(500);
        directoryEvents.push_back(event);
#endif
        break;

    case FILE_ACTION_REMOVED:
        emit q->fileDeleted(name);
        break;

    case FILE_ACTION_RENAMED_NEW_NAME:
        emit q->fileRenamed(oldFileName, name);
        break;

    case FILE_ACTION_RENAMED_OLD_NAME:
        oldFileName = name;
        break;

    default:
        qDebug() << u"Some error, notify->Action" << notify->Action;
        break;
    }
}

WindowsWatcher::WindowsWatcher()
    : DirectoryWatcher(new WindowsWatcherPrivate(this))
{
    Q_D(WindowsWatcher);

    QThread *wkThrd = d->workerThread.data();

    connect(wkThrd, &QThread::started, d->worker.data(), &WatcherWorker::run);
    d->worker.data()->moveToThread(wkThrd);

    auto *windowsWorker = reinterpret_cast<WindowsWorker *>(d->worker.data());

    connect(windowsWorker, &WindowsWorker::finished, wkThrd, &QThread::quit);
    connect(windowsWorker, &WindowsWorker::started,  this,   &WindowsWatcher::observingStarted);
    connect(windowsWorker, &WindowsWorker::finished, this,   &WindowsWatcher::observingStopped);
}

WindowsWatcher::WindowsWatcher(QString const & path)
    : DirectoryWatcher(new WindowsWatcherPrivate(this))
{
    Q_D(WindowsWatcher);
    setWatchPath(path);
}

void WindowsWatcher::setWatchPath(QString path)
{
    Q_D(WindowsWatcher);

    DirectoryWatcher::setWatchPath(path);
    HANDLE hDirectory = d->requestDirectoryHandle(path);
    if (hDirectory == INVALID_HANDLE_VALUE) {
        qDebug() << u"requestDirectoryHandle: INVALID_HANDLE_VALUE";
        return;
    }

    auto *windowsWorker = reinterpret_cast<WindowsWorker *>(d->worker.data());
    windowsWorker->setDirectoryHandle(hDirectory);
}
