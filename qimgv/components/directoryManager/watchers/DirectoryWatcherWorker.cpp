#include "DirectoryWatcherWorker.h"
#include <QDebug>

DirectoryWatcherWorker::DirectoryWatcherWorker() = default;

bool DirectoryWatcherWorker::setRunning(bool running)
{
    return isRunning.exchange(running, std::memory_order::seq_cst);
}
