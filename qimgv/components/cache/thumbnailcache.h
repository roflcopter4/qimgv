#pragma once

#include <QObject>
#include <QDir>
#include <QMutex>
#include <QDebug>
#include "settings.h"
#include "sourcecontainers/thumbnail.h"

class ThumbnailCache : public QObject
{
    Q_OBJECT
  public:
    explicit ThumbnailCache();

    void    saveThumbnail(QImage const *image, QString const &id) const;
    QImage *readThumbnail(QString const &id) const;
    QString thumbnailPath(QString const &id) const;
    bool    exists(QString const &id) const;

  private:
    // we are still bottlenecked by disk access anyway
    QMutex  mutex;
    QString cacheDirPath;
};
