#pragma once

#include "ScalerRequest.h"
#include "ScalerRunnable.h"
#include "components/cache/Cache.h"
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QThreadPool>

class Scaler : public QObject
{
    Q_OBJECT

  public:
    explicit Scaler(QSharedPointer<Cache> cache, QObject *parent);
    ~Scaler() override;

  Q_SIGNALS:
    void scalingFinished(QPixmap *result, ScalerRequest request);
    void acceptScalingResult(QImage *image, ScalerRequest req);
    void startBufferedRequest();

  public Q_SLOTS:
    void requestScaled(ScalerRequest const &req);

  private Q_SLOTS:
    void onTaskStart(ScalerRequest const &req);
    void onTaskFinish(QImage *scaled, ScalerRequest const &req);
    void slotStartBufferedRequest();
    void slotForwardScaledResult(QImage *image, ScalerRequest const &req);

  private:
    QSharedPointer<Cache> cache;
    QThreadPool          *pool;
    ScalerRunnable       *runnable;
    QSemaphore           *sem;

    ScalerRequest bufferedRequest;
    ScalerRequest startedRequest;
    clock_t       currentRequestTimestamp;
    bool          buffered;
    bool          running;

    void startRequest(ScalerRequest const &req);
};
