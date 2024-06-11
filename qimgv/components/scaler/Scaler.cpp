#include "Scaler.h"

/* What this should do in theory:
 * 1 request comes
 * 2 we run it
 * 3a if during scaling no new requests came, we return the result and forget about it. end.
 * 3b if some requests did come, by the end of current task we dispose of its result,
 *    start the last task that came and ignore the middle ones.
 */

Scaler::Scaler(Cache *cache, QObject *parent)
    : QObject(parent),
      pool(new QThreadPool(this)),
      runnable(new ScalerRunnable()),
      sem(new QSemaphore(1)),
      cache(cache),
      currentRequestTimestamp(0),
      buffered(false),
      running(false)
{
    pool->setMaxThreadCount(1);
    runnable->setAutoDelete(false);
    connect(this, &Scaler::startBufferedRequest, this, &Scaler::slotStartBufferedRequest, Qt::DirectConnection);
    connect(runnable, &ScalerRunnable::started, this, &Scaler::onTaskStart, Qt::DirectConnection);
    connect(runnable, &ScalerRunnable::finished, this, &Scaler::onTaskFinish, Qt::DirectConnection);
    connect(this, &Scaler::acceptScalingResult, this, &Scaler::slotForwardScaledResult, Qt::QueuedConnection);
}

Scaler::~Scaler()
{
    delete runnable;
    delete sem;
}

void Scaler::requestScaled(ScalerRequest const &req)
{
    sem->acquire(1);
    if (!running) {
        //////////////////////////////////
        if (!buffered) {
            bufferedRequest = req;
            buffered        = true;
            // qDebug() << "1 requestScaled() - locking..  " <<  req.image->name();
            cache->reserve(req.image->fileName());
            // qDebug() << "1 requestScaled() - LOCKED!  " <<  req.image->name();
            startRequest(req);
        } else if (bufferedRequest.image != req.image) {
            // qDebug() << "2 requestScaled() - locking...  " <<  req.image->name();
            cache->reserve(req.image->fileName());
            // qDebug() << "2 requestScaled() - LOCKED!  " <<  req.image->name();
            auto tmp        = bufferedRequest;
            bufferedRequest = req;
            buffered        = true;
            if (startedRequest.image != tmp.image) {
                cache->release(tmp.image->fileName());
                // qDebug() << "2 requestScaled() - RELEASED!  " <<  tmp.image->name();
            }
        } else {
            bufferedRequest = req;
            buffered        = true;
        }
        //////////////////////////////
    } else if (!buffered) {
        if (req.image != startedRequest.image)
            cache->reserve(req.image->fileName());
        bufferedRequest = req;
        buffered        = true;
    } else if (req.image == bufferedRequest.image) {
        bufferedRequest = req;
        buffered        = true;
    } else {
        if (bufferedRequest.image != startedRequest.image) {
            // qDebug() << "4 RELEASING " << bufferedRequest.image->name();
            cache->release(bufferedRequest.image->fileName());
        }
        if (req.image != startedRequest.image)
            cache->reserve(req.image->fileName());
        bufferedRequest = req;
        buffered        = true;
    }
    sem->release(1);
}

void Scaler::onTaskStart(ScalerRequest const &req)
{
    sem->acquire(1);
    running = true;
    // clear buffered flag if there were no requests after us
    if (buffered && bufferedRequest == req)
        buffered = false;
    startedRequest = req;
    // qDebug() << "onTaskStart(): " << req.image->name();
    sem->release(1);
}

void Scaler::onTaskFinish(QImage *scaled, ScalerRequest const &req)
{
    sem->acquire(1);
    running = false;
    if (buffered && bufferedRequest.image == req.image) {
    } else {
        // qDebug() << "onTaskFinish() - 2 releasing..  " <<  req.image->name();
        QString name = req.image->fileName();
        cache->release(req.image->fileName());
        // qDebug() << "onTaskFinish() - 2 RELEASED!  " <<  name;
    }
    if (buffered) {
        // qDebug() << "onTaskFinish - startingBuffered: " << bufferedRequest.string;
        delete scaled;
        // startRequest(bufferedRequest);
        emit startBufferedRequest();
        sem->release(1);
    } else {
        sem->release(1);
        emit acceptScalingResult(scaled, req);
    }
}

void Scaler::slotStartBufferedRequest()
{
    startRequest(bufferedRequest);
}

void Scaler::slotForwardScaledResult(QImage *image, ScalerRequest const &req)
{
    QPixmap *pixmap = new QPixmap();
    *pixmap         = QPixmap::fromImage(*image);
    delete image;
    emit scalingFinished(pixmap, req);
}

void Scaler::startRequest(ScalerRequest const &req)
{
    runnable->setRequest(req);
    pool->start(runnable);
}
