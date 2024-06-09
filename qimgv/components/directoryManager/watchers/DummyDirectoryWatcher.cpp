#include "DummyDirectoryWatcher.h"
#include "DirectoryWatcher_p.h"
#include <QDebug>

static constexpr char16_t tag[]     = u"[DummyDirectoryWatcher]";
static constexpr char16_t message[] = u"Directory watcher isn't yet implemented for your operating system";

class DummyDirectoryWatcherWorker final : public DirectoryWatcherWorker
{
  public:
    DummyDirectoryWatcherWorker() = default;

    void run() override
    {
        qDebug() << tag << message;
    }
};

class DummyDirectoryWatcherPrivate final : public DirectoryWatcherPrivate
{
  public:
    explicit DummyDirectoryWatcherPrivate(DirectoryWatcher *watcher)
        : DirectoryWatcherPrivate(watcher, new DummyDirectoryWatcherWorker())
    {
    }
};

DummyDirectoryWatcher::DummyDirectoryWatcher(DirectoryManager *parent)
    : DirectoryWatcher(new DummyDirectoryWatcherPrivate(this), parent)
{
    qDebug() << tag << message;
}
