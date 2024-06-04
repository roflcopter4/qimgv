#include "loaderrunnable.h"

#include <QElapsedTimer>

LoaderRunnable::LoaderRunnable(QString path)
    : path_(std::move(path))
{}

void LoaderRunnable::run()
{
    //QElapsedTimer t;
    //t.start();
    auto image = ImageFactory::createImage(path_);
    //qDebug() << u"L:" << t.elapsed();
    emit finished(std::move(image), path_);
}
