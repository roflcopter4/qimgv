#pragma once

#include "../DirectoryWatcher.h"

class DirectoryManager;
class LinuxDirectoryWatcherPrivate;

class LinuxDirectoryWatcher : public DirectoryWatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LinuxDirectoryWatcher)

  public:
    explicit LinuxDirectoryWatcher(DirectoryManager *parent);
    ~LinuxDirectoryWatcher() override;
};
