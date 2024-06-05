#pragma once

#include "LinuxFsEvent.h"
#include "../WatcherWorker.h"

class LinuxWorker : public WatcherWorker
{
    Q_OBJECT
public:
    LinuxWorker();

    void setDescriptor(int desc);
    void handleErrorCode(int code);

    virtual void run() override;

signals:
    void fileEvent(LinuxFsEvent* event);

private:
    int fd;
};
