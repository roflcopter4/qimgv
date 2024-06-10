#pragma once

#include <QObject>
#include <QDir>
#include <QMutex>
#include <QDebug>
#include "Settings.h"
#include "sourcecontainers/Thumbnail.h"

class ThumbnailCache : public QObject
{
    Q_OBJECT

  public:
    explicit ThumbnailCache(QObject *parent);

    bool    saveThumbnail(QImage const *image, QString const &id) const;
    QImage *readThumbnail(QString const &id) const;
    QString thumbnailPath(QString const &id) const;
    bool    exists(QString const &id) const;

  private:
    // we are still bottlenecked by disk access anyway
    QMutex  mutex;
    QString cacheDirPath;
};
