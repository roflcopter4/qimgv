#pragma once

#include "../directorywatcher.h"

class LinuxWatcherPrivate;

class LinuxWatcher : public DirectoryWatcher {
    Q_OBJECT
public:
    explicit     LinuxWatcher();
    virtual      ~LinuxWatcher();
    virtual void setWatchPath(QString p);

private:
    Q_DECLARE_PRIVATE(LinuxWatcher)
};
