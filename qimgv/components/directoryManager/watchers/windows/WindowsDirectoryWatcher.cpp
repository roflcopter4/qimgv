#include "WindowsDirectoryWatcher.h"
#include "WindowsDirectoryWatcher_p.h"
#include "WindowsDirectoryWorker.h"
#include "utils/Stuff.h"

/****************************************************************************************/

WindowsDirectoryWatcherPrivate::WindowsDirectoryWatcherPrivate(DirectoryWatcher *qq)
    : DirectoryWatcherPrivate(qq, new WindowsDirectoryWorker())
{
    auto *windowsWorker = reinterpret_cast<WindowsDirectoryWorker*>(worker.get());
    qRegisterMetaType<PFILE_NOTIFY_INFORMATION>("PFILE_NOTIFY_INFORMATION");

    connect(windowsWorker, SIGNAL(notifyEvent(PFILE_NOTIFY_INFORMATION)), this, SLOT(dispatchNotify(PFILE_NOTIFY_INFORMATION)));

    connect(workerThread.get(), &QThread::started, worker.get(), &DirectoryWatcherWorker::run);
    worker->moveToThread(workerThread.get());
    connect(worker.get(), &DirectoryWatcherWorker::finished, workerThread.get(), &QThread::quit);

}

void WindowsDirectoryWatcherPrivate::setWatchPath(QString path)
{
    auto *windowsWorker = reinterpret_cast<WindowsDirectoryWorker *>(worker.get());

    if (path.isEmpty()) {
        windowsWorker->setDirectoryHandle(INVALID_HANDLE_VALUE);
    } else {
        HANDLE hDirectory = requestDirectoryHandle(path);
        if (hDirectory == INVALID_HANDLE_VALUE) {
            qDebug() << u"requestDirectoryHandle: INVALID_HANDLE_VALUE";
            return;
        }
        windowsWorker->setDirectoryHandle(hDirectory);
    }

    DirectoryWatcherPrivate::setWatchPath(std::move(path));
}

HANDLE WindowsDirectoryWatcherPrivate::requestDirectoryHandle(QString const &path)
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
                qDebug() << u"Error in requestDirectoryHandle:" <<  util::GetErrorMessage(err);
                return INVALID_HANDLE_VALUE;
            }
        }
    } while (hDirectory == INVALID_HANDLE_VALUE);

    return hDirectory;
}

void WindowsDirectoryWatcherPrivate::dispatchNotify(PFILE_NOTIFY_INFORMATION notify)
{
    Q_Q(WindowsDirectoryWatcher);

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

/****************************************************************************************/

WindowsDirectoryWatcher::WindowsDirectoryWatcher(DirectoryManager *parent)
    : DirectoryWatcher(new WindowsDirectoryWatcherPrivate(this), parent)
{}

