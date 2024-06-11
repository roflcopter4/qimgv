#include <QTimer>

#include <sys/inotify.h>

#include "LinuxDirectoryWatcher_p.h"
#include "LinuxDirectoryWorker.h"
#include "utils/Stuff.h"

#define TAG "[LinuxDirectoryWatcher]"
static constexpr uint32_t INOTIFY_EVENT_MASK = IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVE;

/**
 * Time to wait for rename event. If event take time longer
 * than specified then event will be considered as remove event
 */

// TODO: this may break event order.
// Implement a proper queue.
static constexpr int EVENT_MOVE_TIMEOUT   = 150; // ms;
static constexpr int EVENT_MODIFY_TIMEOUT = 150; // ms;

/****************************************************************************************/

LinuxDirectoryWatcherPrivate::LinuxDirectoryWatcherPrivate(LinuxDirectoryWatcher *qq)
    : DirectoryWatcherPrivate(qq, new LinuxDirectoryWorker()),
      watcher(inotify_init()),
      watchObject(-1)
{
    connect(workerThread.get(), &QThread::started, worker.get(), &DirectoryWatcherWorker::run);
    worker->moveToThread(workerThread.get());

    auto *linuxWorker = static_cast<LinuxDirectoryWorker *>(worker.get());
    linuxWorker->setDescriptor(watcher);
    connect(linuxWorker, &LinuxDirectoryWorker::fileEvent, this, &LinuxDirectoryWatcherPrivate::dispatchFilesystemEvent);

    // There's no need to destroy thread and worker. They're will be removed automatically
    connect(worker.get(), &LinuxDirectoryWorker::finished, workerThread.get(), &QThread::quit);
}

LinuxDirectoryWatcherPrivate::~LinuxDirectoryWatcherPrivate()
{
    if (watchObject != -1) {
        int removeStatusCode = inotify_rm_watch(watcher, watchObject);
        if (removeStatusCode == 0)
            watchObject = -1;
        else
            qDebug() << TAG << "Cannot remove inotify watcher instance:" << util::GetErrorMessage(errno);
    }
    workerThread->quit();
}

void LinuxDirectoryWatcherPrivate::setWatchPath(QString path)
{
    // Subscribe for specified filesystem events
    if (watchObject != -1) {
        int status = inotify_rm_watch(watcher, watchObject);
        if (status == -1)
            qDebug() << TAG << "Cannot remove inotify watcher instance:" << util::GetErrorMessage(errno);
    }

    if (!path.isEmpty()) {
        // Add new path to be watched by inotify
        watchObject = inotify_add_watch(watcher, path.toUtf8(), INOTIFY_EVENT_MASK);
        if (watchObject == -1)
            qDebug() << TAG << "Error:" << util::GetErrorMessage(errno);
    }

    DirectoryWatcherPrivate::setWatchPath(std::move(path));
}

qsizetype LinuxDirectoryWatcherPrivate::indexOfWatcherEvent(uint cookie) const
{
    for (qsizetype i = 0; i < watcherEvents.size(); ++i) {
        auto event = watcherEvents.at(i);
        if (event->cookie() == cookie)
            return i;
    }
    return -1;
}

qsizetype LinuxDirectoryWatcherPrivate::indexOfWatcherEvent(QString const &name) const
{
    for (qsizetype i = 0; i < watcherEvents.size(); ++i) {
        auto event = watcherEvents.at(i);
        if (event->name() == name)
            return i;
    }
    return -1;
}

void LinuxDirectoryWatcherPrivate::dispatchFilesystemEvent(LinuxFsEvent *e)
{
    Q_Q(LinuxDirectoryWatcher);
    std::unique_ptr<LinuxFsEvent> event(e);

    uint dataOffset = 0;

    while (dataOffset < event->dataSize()) {
        auto *notify_event = reinterpret_cast<inotify_event *>(event->data() + dataOffset);
        dataOffset += sizeof(inotify_event) + notify_event->len;

        uint32_t mask       = notify_event->mask;
        QString  name       = QString::fromUtf8(notify_event->name);
        uint     cookie     = notify_event->cookie;
        bool     isDirEvent = mask & IN_ISDIR;

        // Skip events for directories and files that isn't in filter range
        /*if((isDirEvent) && !(mask & IN_MOVED_TO) ) {
            continue;
        }*/

        if (mask & IN_MODIFY)
            handleModifyEvent(name);
        else if (mask & IN_CREATE)
            handleCreateEvent(name);
        else if (mask & IN_DELETE)
            handleDeleteEvent(name);
        else if (mask & IN_MOVED_FROM)
            handleMovedFromEvent(name, cookie);
        else if (mask & IN_MOVED_TO)
            handleMovedToEvent(name, cookie);
    }
}

void LinuxDirectoryWatcherPrivate::handleModifyEvent(QString const &name)
{
    // Find the same event in the list by file name
    qsizetype eventIndex = indexOfWatcherEvent(name);
    if (eventIndex == -1) {
        // This is this first modify event for the current file
        int   timerId = startTimer(EVENT_MODIFY_TIMEOUT);
        auto *event   = new WatcherEvent(name, timerId, WatcherEvent::Type::Modify);
        watcherEvents.append(QSharedPointer<WatcherEvent>(event));
    } else {
        auto event = watcherEvents.at(eventIndex);
        // Restart timer again
        killTimer(event->timerId());
        int timerId = startTimer(EVENT_MODIFY_TIMEOUT);
        event->setTimerId(timerId);
    }
}

void LinuxDirectoryWatcherPrivate::handleDeleteEvent(QString const &name)
{
    Q_Q(LinuxDirectoryWatcher);
    emit q->fileDeleted(name);
}

void LinuxDirectoryWatcherPrivate::handleCreateEvent(QString const &name)
{
    Q_Q(LinuxDirectoryWatcher);
    emit q->fileCreated(name);
}

void LinuxDirectoryWatcherPrivate::handleMovedFromEvent(QString const &name, uint cookie)
{
    int timerId = startTimer(EVENT_MOVE_TIMEOUT);
    // Save timer id to find out later which event timer is running
    auto *event = new WatcherEvent(name, cookie, timerId, WatcherEvent::Type::MovedFrom);
    watcherEvents.append(QSharedPointer<WatcherEvent>(event));
}

void LinuxDirectoryWatcherPrivate::handleMovedToEvent(QString const &name, uint cookie)
{
    Q_Q(LinuxDirectoryWatcher);

    // Check if file waiting to be renamed
    qsizetype eventIndex = indexOfWatcherEvent(cookie);
    if (eventIndex == -1) {
        // No one event waiting for rename so this is a new file
        emit q->fileCreated(name);
    } else {
        // Waiting for rename event is found
        auto watcherEvent = watcherEvents.takeAt(eventIndex);
        // Kill associated timer
        killTimer(watcherEvent->timerId());
        emit q->fileRenamed(watcherEvent->name(), name);
    }
}

void LinuxDirectoryWatcherPrivate::timerEvent(QTimerEvent *timerEvent)
{
    Q_Q(LinuxDirectoryWatcher);

    // Loop through waiting move events
    qsizetype lastIndex = watcherEvents.size() - 1;
    for (qsizetype i = lastIndex; i >= 0; --i) {
        auto watcherEvent = watcherEvents.at(i);

        if (watcherEvent->timerId() == timerEvent->timerId()) {
            auto type = watcherEvent->type();
            if (type == WatcherEvent::Type::MovedFrom) {
                // Rename event didn't happen so treat this event as remove event
                emit q->fileDeleted(watcherEvent->name());
            } else if (type == WatcherEvent::Type::Modify) {
                emit q->fileModified(watcherEvent->name());
            }

            watcherEvents.removeAt(i);
            break;
        }
    }

    // Stop timer anyway
    killTimer(timerEvent->timerId());
}

/****************************************************************************************/

LinuxDirectoryWatcher::LinuxDirectoryWatcher(DirectoryManager *parent)
    : DirectoryWatcher(new LinuxDirectoryWatcherPrivate(this), parent)
{}

LinuxDirectoryWatcher::~LinuxDirectoryWatcher() = default;
