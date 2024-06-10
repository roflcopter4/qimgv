#pragma once

#include <QObject>
#include <QThreadPool>
#include <QThread>
#include <QMutex>
#include "components/cache/Cache.h"
#include "ScalerRequest.h"
#include "ScalerRunnable.h"

class Scaler : public QObject {
    Q_OBJECT
public:
    explicit Scaler(Cache *_cache, QObject *parent = nullptr);

signals:
    void scalingFinished(QPixmap* result, ScalerRequest request);
    void acceptScalingResult(QImage *image, ScalerRequest req);
    void startBufferedRequest();

public slots:
    void requestScaled(ScalerRequest const &req);

private slots:
    void onTaskStart(ScalerRequest const &req);
    void onTaskFinish(QImage*scaled, ScalerRequest const &req);
    void slotStartBufferedRequest();
    void slotForwardScaledResult(QImage *image, ScalerRequest const &req);

private:
    QThreadPool *pool;
    ScalerRunnable *runnable;
    bool buffered, running;
    clock_t currentRequestTimestamp;
    ScalerRequest bufferedRequest, startedRequest;

    Cache *cache;

    void startRequest(ScalerRequest const &req);

    QSemaphore *sem;
};
