#pragma once

#include "../DirectoryWatcher.h"

class DirectoryManager;
class LinuxDirectoryWatcherPrivate;

class LinuxDirectoryWatcher final : public DirectoryWatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LinuxDirectoryWatcher)

  public:
    explicit LinuxDirectoryWatcher(DirectoryManager *parent);
    ~LinuxDirectoryWatcher() override;
    DELETE_COPY_MOVE_ROUTINES(LinuxDirectoryWatcher);
};
