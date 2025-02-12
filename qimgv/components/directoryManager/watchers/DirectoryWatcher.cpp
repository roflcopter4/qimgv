#include "../DirectoryManager.h"
#include "DirectoryWatcher_p.h"

#if defined Q_OS_LINUX || defined Q_OS_FREEBSD
# include "linux/LinuxDirectoryWatcher.h"
using DirectoryWatcherImplementation = LinuxDirectoryWatcher;
#elif defined Q_OS_WINDOWS
# include "windows/WindowsDirectoryWatcher.h"
using DirectoryWatcherImplementation = WindowsDirectoryWatcher;
#elif defined Q_OS_APPLE
  // TODO: implement this
# include "DummyDirectoryWatcher.h"
using DirectoryWatcherImplementation = DummyDirectoryWatcher;
#elif defined Q_OS_UNIX
  // TODO: implement this
# include "DummyDirectoryWatcher.h"
using DirectoryWatcherImplementation = DummyDirectoryWatcher;
#else
  // TODO: implement this
# include "DummyDirectoryWatcher.h"
using DirectoryWatcherImplementation = DummyDirectoryWatcher;
#endif

/****************************************************************************************/

DirectoryWatcherPrivate::DirectoryWatcherPrivate(DirectoryWatcher *qq, DirectoryWatcherWorker *w)
    : q_ptr(qq),
      worker(w),
      workerThread(new QThread(this))
{
    workerThread->setObjectName(u"Directory Watcher Worker"_sv);
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
    connect(d->worker.get(), &DirectoryWatcherWorker::started,  this, &DirectoryWatcher::observingStarted);
    connect(d->worker.get(), &DirectoryWatcherWorker::finished, this, &DirectoryWatcher::observingStopped);
}

DirectoryWatcher::~DirectoryWatcher()
{
    stopObserving();
    DirectoryWatcher::setWatchPath(QString());
}

// Move this function to some creational class
DirectoryWatcher *DirectoryWatcher::newInstance(DirectoryManager *parent)
{
    DirectoryWatcher *watcher = new DirectoryWatcherImplementation(parent);
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

