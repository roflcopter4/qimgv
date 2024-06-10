#pragma once

#include "LinuxFsEvent.h"
#include "../DirectoryWatcherWorker.h"

class LinuxDirectoryWorker : public DirectoryWatcherWorker
{
    Q_OBJECT

  public:
    LinuxDirectoryWorker();

    void setDescriptor(int desc);
    void handleErrorCode(ssize_t code);

    virtual void run() override;

  Q_SIGNALS:
    void fileEvent(LinuxFsEvent *event);

  private:
    int fd;
};
