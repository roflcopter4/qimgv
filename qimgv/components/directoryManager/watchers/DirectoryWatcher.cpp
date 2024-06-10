#include "../DirectoryManager.h"
#include "DirectoryWatcher_p.h"

#if defined __linux__
# include "linux/LinuxDirectoryWatcher.h"
#elif defined Q_OS_WIN32
# include "windows/WindowsDirectoryWatcher.h"
#elif defined Q_OS_APPLE
  // TODO: implement this
# include "DummyDirectoryWatcher.h"
#elif defined Q_OS_UNIX
  // TODO: implement this
# include "DummyDirectoryWatcher.h"
#else
  // TODO: implement this
# include "DummyDirectoryWatcher.h"
#endif

/****************************************************************************************/

DirectoryWatcherPrivate::DirectoryWatcherPrivate(DirectoryWatcher *qq, DirectoryWatcherWorker *w)
    : q_ptr(qq),
      worker(w),
      workerThread(new QThread(this))
{
}

DirectoryWatcherPrivate::~DirectoryWatcherPrivate()
{
    workerThread->wait();
}

void DirectoryWatcherPrivate::setWatchPath(QString path)
{
    currentDirectory = std::move(path);
}

/****************************************************************************************/

DirectoryWatcher::DirectoryWatcher(DirectoryWatcherPrivate *ptr, DirectoryManager *parent)
    : QObject(parent),
      d_ptr(ptr)
{
    Q_D(DirectoryWatcher);
    connect(d->worker.get(), &DirectoryWatcherWorker::started,  this,   &DirectoryWatcher::observingStarted);
    connect(d->worker.get(), &DirectoryWatcherWorker::finished, this,   &DirectoryWatcher::observingStopped);
}

DirectoryWatcher::~DirectoryWatcher()
{
    stopObserving();
    DirectoryWatcher::setWatchPath(QString());
}

// Move this function to some creational class
DirectoryWatcher *DirectoryWatcher::newInstance(DirectoryManager *parent)
{
    DirectoryWatcher *watcher;

#if defined Q_OS_LINUX
    watcher = new LinuxDirectoryWatcher(parent);
#elif defined Q_OS_WIN32
    watcher = new WindowsDirectoryWatcher(parent);
#elif defined Q_OS_APPLE
      watcher = new DummyDirectoryWatcher(parent);
#elif defined Q_OS_UNIX
      watcher = new DummyDirectoryWatcher(parent);
#else
      watcher = new DummyDirectoryWatcher(parent);
#endif

    return watcher;
}

void DirectoryWatcher::setWatchPath(QString const &path)
{
    Q_D(DirectoryWatcher);
    d->setWatchPath(path);
}

QString DirectoryWatcher::watchPath() const
{
    Q_D(const DirectoryWatcher);
    return d->currentDirectory;
}

void DirectoryWatcher::observe()
{
    Q_D(DirectoryWatcher);
    if (!isObserving()) {
        // Reuse worker instance
        d->worker->setRunning(true);
        d->workerThread->start();
    }
    //qDebug() << TAG << "Observing path:" << d->currentDirectory;
}

void DirectoryWatcher::stopObserving()
{
    Q_D(DirectoryWatcher);
    d->worker->setRunning(false);
}

bool DirectoryWatcher::isObserving() const
{
    Q_D(const DirectoryWatcher);
    return d->workerThread->isRunning();
}

