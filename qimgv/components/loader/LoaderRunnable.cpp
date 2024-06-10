#include "LoaderRunnable.h"

#include <QElapsedTimer>

LoaderRunnable::LoaderRunnable(QString path)
    : path_(std::move(path))
{}

void LoaderRunnable::run()
{
    emit finished(ImageFactory::createImage(path_), path_);
}
