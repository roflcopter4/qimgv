#include "dummywatcher.h"
#include "directorywatcher_p.h"
#include <QDebug>

static constexpr QStringView tag     = QSV("[DummyWatcher]");
static constexpr QStringView message = QSV("Directory watcher isn't yet implemented for your operating system");

class DummyWatcherWorker : public WatcherWorker {
  public:
    DummyWatcherWorker() {}

    void run() override {
        qDebug() << tag << message;
    }
};

class DummyWatcherPrivate : public DirectoryWatcherPrivate {
  public:
    DummyWatcherPrivate(DirectoryWatcher* watcher) : DirectoryWatcherPrivate(watcher, new DummyWatcherWorker()) {}
};

DummyWatcher::DummyWatcher() : DirectoryWatcher(new DummyWatcherPrivate(this))
{
    qDebug() << tag << message;
}
