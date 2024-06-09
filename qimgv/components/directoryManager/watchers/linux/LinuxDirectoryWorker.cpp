#include <sys/poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <QThread>
#include <QDebug>

#include "LinuxDirectoryWorker.h"
#include "utils/Stuff.h"

#define TAG "[LinuxWatcherWorker]"
static constexpr int TIMEOUT = 300; // ms

LinuxDirectoryWorker::LinuxWorker()
    : fd(-1)
{
}

void LinuxDirectoryWorker::setDescriptor(int desc)
{
    fd = desc;
}

void LinuxDirectoryWorker::run()
{
    emit started();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    isRunning.store(true);
#else
    isRunning.storeRelaxed(true);
#endif

    if (fd == -1) {
        qDebug() << TAG << u"File descriptor isn't set! Stopping";
        emit finished();
        return;
    }

    while (isRunning) {
        int  errorCode      = 0;
        uint bytesAvailable = 0;

        // File descriptor event struct for polling service
        pollfd pollDescriptor = {fd, POLLIN, 0};

        // Freeze thread till next event
        errorCode = poll(&pollDescriptor, 1, TIMEOUT);
        handleErrorCode(errorCode);

        // Check how many bytes available
        errorCode = ioctl(fd, FIONREAD, &bytesAvailable);
        handleErrorCode(errorCode);

        if (bytesAvailable == 0)
            continue;

        char *eventData = new char[bytesAvailable];
        errorCode       = read(fd, eventData, bytesAvailable);
        handleErrorCode(errorCode);

        emit fileEvent(new LinuxFsEvent(eventData, bytesAvailable));
    }

    emit finished();
}

void LinuxDirectoryWorker::handleErrorCode(int code)
{
    if (code == -1) {
        QString err = util::GetErrorMessage(errno);
        qDebug() << TAG << u"Error:" << err;
        emit error(err);
    }
}
