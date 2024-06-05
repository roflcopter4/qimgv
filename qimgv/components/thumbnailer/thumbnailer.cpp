#include "thumbnailer.h"

Thumbnailer::Thumbnailer()
{
    cache = new ThumbnailCache();
    pool  = new QThreadPool(this);

    int threads       = settings->thumbnailerThreadCount();
    int globalThreads = QThreadPool::globalInstance()->maxThreadCount();
    if (threads > globalThreads)
        threads = globalThreads;
    pool->setMaxThreadCount(threads);
}

Thumbnailer::~Thumbnailer()
{
    pool->clear();
    pool->waitForDone();
}

void Thumbnailer::waitForDone()
{
    pool->waitForDone();
}

void Thumbnailer::clearTasks()
{
    pool->clear();
}

QSharedPointer<Thumbnail> Thumbnailer::getThumbnail(QString const &filePath, int size)
{
    return ThumbnailerRunnable::generate(nullptr, filePath, size, false, false);
}

void Thumbnailer::getThumbnailAsync(QString const &path, int size, bool crop, bool force)
{
    if (!runningTasks.contains(path, size))
        startThumbnailerThread(path, size, crop, force);
}

void Thumbnailer::startThumbnailerThread(QString const &filePath, int size, bool crop, bool force)
{
    auto runnable = new ThumbnailerRunnable(settings->useThumbnailCache() ? cache : nullptr, filePath, size, crop, force);
    connect(runnable, &ThumbnailerRunnable::taskStart, this, &Thumbnailer::onTaskStart);
    connect(runnable, &ThumbnailerRunnable::taskEnd, this, &Thumbnailer::onTaskEnd);
    runnable->setAutoDelete(true);
    pool->start(runnable);
}

void Thumbnailer::onTaskStart(QString const &filePath, int size)
{
    runningTasks.insert(filePath, size);
}

void Thumbnailer::onTaskEnd(QSharedPointer<Thumbnail> const &thumbnail, QString const &filePath)
{
    runningTasks.remove(filePath, thumbnail->size());
    emit thumbnailReady(thumbnail, filePath);
}
