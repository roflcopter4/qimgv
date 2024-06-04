#include "loader.h"

Loader::Loader()
{
    pool = new QThreadPool(this);
    pool->setMaxThreadCount(2);
}

void Loader::clearTasks()
{
    clearPool();
    pool->waitForDone();
}

bool Loader::isBusy() const
{
    return (tasks.count() != 0);
}

bool Loader::isLoading(QString const &path) const
{
    return tasks.contains(path);
}

std::shared_ptr<Image> Loader::load(QString const &path)
{
    return ImageFactory::createImage(path);
}

// clears all buffered tasks before loading
void Loader::loadAsyncPriority(QString const &path)
{
    clearPool();
    doLoadAsync(path, 1);
}

void Loader::loadAsync(QString const &path)
{
    doLoadAsync(path, 0);
}

void Loader::doLoadAsync(QString const &path, int priority)
{
    if (tasks.contains(path))
        return;

    auto runnable = new LoaderRunnable(path);
    runnable->setAutoDelete(false);
    tasks.insert(path, runnable);
    connect(runnable, &LoaderRunnable::finished, this, &Loader::onLoadFinished, Qt::UniqueConnection);
    pool->start(runnable, priority);
}

void Loader::onLoadFinished(std::shared_ptr<Image> const &image, QString const &path)
{
    auto task = tasks.take(path);
    delete task;
    if (!image)
        emit loadFailed(path);
    else
        emit loadFinished(image, path);
}

void Loader::clearPool()
{
    QHashIterator i(tasks);
    while (i.hasNext()) {
        i.next();
        if (pool->tryTake(i.value()))
            delete tasks.take(i.key());
    }
}
