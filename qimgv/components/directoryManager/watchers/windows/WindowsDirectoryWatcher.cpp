#include "WindowsDirectoryWatcher.h"
#include "WindowsDirectoryWatcher_p.h"
#include "WindowsDirectoryWorker.h"
#include "utils/Stuff.h"

/****************************************************************************************/

WindowsDirectoryWatcherPrivate::WindowsDirectoryWatcherPrivate(DirectoryWatcher *qq)
    : DirectoryWatcherPrivate(qq, new WindowsDirectoryWorker())
{
    auto *workerPtr = reinterpret_cast<WindowsDirectoryWorker *>(worker.get());
    auto *threadPtr = workerThread.get();

    connect(threadPtr, &QThread::started,                    workerPtr, &DirectoryWatcherWorker::run);
    connect(workerPtr, &WindowsDirectoryWorker::notifyEvent, this,      &WindowsDirectoryWatcherPrivate::dispatchNotify);
    connect(workerPtr, &DirectoryWatcherWorker::finished,    threadPtr, &QThread::quit);

    workerPtr->moveToThread(threadPtr);
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
    std::wstring wpath = path.startsWith(uR"(\\?\)"_sv)
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

void WindowsDirectoryWatcherPrivate::dispatchNotify(LPBYTE ptr)
{
    auto    sptr  = std::unique_ptr<BYTE[]>(ptr);
    auto   *event = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(ptr);
    DWORD   len   = event->FileNameLength / sizeof(WCHAR);
    QString name  = QString::fromWCharArray(event->FileName, len);

    Q_Q(WindowsDirectoryWatcher);

    switch (event->Action) {
    case FILE_ACTION_ADDED:            emit q->fileCreated(name);              break;
    case FILE_ACTION_MODIFIED:         emit q->fileModified(name);             break;
    case FILE_ACTION_REMOVED:          emit q->fileDeleted(name);              break;
    case FILE_ACTION_RENAMED_NEW_NAME: emit q->fileRenamed(oldFileName, name); break;
    case FILE_ACTION_RENAMED_OLD_NAME: oldFileName = std::move(name);          break;
    default:
        qDebug() << u"Some error, notify->Action:" << event->Action;
        break;
    }
}

/****************************************************************************************/

WindowsDirectoryWatcher::WindowsDirectoryWatcher(DirectoryManager *parent)
    : DirectoryWatcher(new WindowsDirectoryWatcherPrivate(this), parent)
{}

