#include "watcherworker.h"
#include <QDebug>

WatcherWorker::WatcherWorker() = default;

bool WatcherWorker::setRunning(bool running)
{
    return isRunning.exchange(running, std::memory_order::seq_cst);
}
