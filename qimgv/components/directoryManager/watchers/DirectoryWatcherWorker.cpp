#include "DirectoryWatcherWorker.h"

DirectoryWatcherWorker::DirectoryWatcherWorker() = default;

void DirectoryWatcherWorker::setRunning(bool running)
{
    isRunning.store(running, std::memory_order::release);
}
