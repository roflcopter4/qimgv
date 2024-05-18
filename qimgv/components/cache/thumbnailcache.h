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

    void    saveThumbnail(QImage *image, QString const &id);
    QImage* readThumbnail(QString const &id);
    QString thumbnailPath(QString const &id);
    bool    exists(QString const &id);

signals:

public slots:

private:
    // we are still bottlenecked by disk access anyway
    QMutex mutex;
    QString cacheDirPath;
};
