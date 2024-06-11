#include <QDebug>
#include <QThread>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>

#include "LinuxDirectoryWorker.h"
#include "utils/Stuff.h"

#define TAG "[LinuxWatcherWorker]"
static constexpr int TIMEOUT = 300; // ms

LinuxDirectoryWorker::LinuxDirectoryWorker()
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
    isRunning.store(true, std::memory_order::relaxed);
#endif

    if (fd == -1) {
        qDebug() << TAG << u"File descriptor isn't set! Stopping.";
        emit finished();
        return;
    }
    if (!isRunning.load(std::memory_order::relaxed)) {
        emit finished();
        return;
    }
    while (true) {
        ssize_t errorCode      = 0;
        uint    bytesAvailable = 0;

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

        auto eventData = std::unique_ptr<char[]>(new char[bytesAvailable + 1]);
        errorCode      = read(fd, eventData.get(), bytesAvailable);
        handleErrorCode(errorCode);
        eventData[bytesAvailable] = '\0';

        if (!isRunning.load(std::memory_order::relaxed))
            break;
        emit fileEvent(new LinuxFsEvent(std::move(eventData), bytesAvailable));
    }

    emit finished();
}

void LinuxDirectoryWorker::handleErrorCode(ssize_t code)
{
    if (code == -1) {
        QString err = util::GetErrorMessage(errno);
        qDebug() << TAG << u"Error:" << err;
        emit error(err);
    }
}
