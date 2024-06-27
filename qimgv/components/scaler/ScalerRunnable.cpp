#include "ScalerRunnable.h"
#include <QElapsedTimer>

ScalerRunnable::ScalerRunnable(QObject *parent)
    : QObject(parent)
{
}

void ScalerRunnable::setRequest(ScalerRequest const &r)
{
    req = r;
}

void ScalerRunnable::run()
{
    emit started(req);
    // QElapsedTimer t;
    // t.start();
    QImage *scaled;
    if (req.filter == ScalingFilter::NEAREST || (req.size.width() > req.image->width() && !settings->smoothUpscaling()))
        scaled = ImageLib::scaled(req.image->getImage(), req.size, ScalingFilter::NEAREST);
    else
        scaled = ImageLib::scaled(req.image->getImage(), req.size, req.filter);
    // qDebug() << ">> " << req.size << ": " << t.elapsed();
    emit finished(scaled, req);
}