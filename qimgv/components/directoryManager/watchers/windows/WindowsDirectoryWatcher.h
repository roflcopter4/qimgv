#pragma once

#include "../DirectoryWatcher.h"

class WindowsDirectoryWatcherPrivate;

class WindowsDirectoryWatcher final : public DirectoryWatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WindowsDirectoryWatcher)

  public:
    explicit WindowsDirectoryWatcher(DirectoryManager *parent);
};
