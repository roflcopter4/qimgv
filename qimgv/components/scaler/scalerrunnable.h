#pragma once

#include <QObject>
#include <QRunnable>
#include <QThread>
#include <QDebug>
#include "components/cache/cache.h"
#include "scalerrequest.h"
#include "utils/imagelib.h"
#include "Settings.h"

class ScalerRunnable : public QObject, public QRunnable
{
    Q_OBJECT

  public:
    explicit ScalerRunnable();
    void setRequest(ScalerRequest const &r);
    void run() override;

  Q_SIGNALS:
    void started(ScalerRequest);
    void finished(QImage *, ScalerRequest);

  private:
    ScalerRequest req;

    static constexpr double CMPL_FALLBACK_THRESHOLD = 70.0; // equivalent of ~ 5000x3500 @ 32bpp
};
