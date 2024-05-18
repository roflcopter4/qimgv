#pragma once

#include <QDebug>
#include <QMap>
#include <QSemaphore>
#include <QMutexLocker>
#include "sourcecontainers/image.h"
#include "components/cache/cacheitem.h"
#include "utils/imagefactory.h"

class Cache {
public:
    explicit Cache();
    bool     contains(QString const &path) const;
    void     remove(QString const &path);
    void     clear();

    bool insert(std::shared_ptr<Image> const &img);
    void trimTo(QStringList const &list);

    std::shared_ptr<Image> get(QString const &path);
    bool                   release(QString const &path);
    bool                   reserve(QString const &path);
    const QList<QString>   keys() const;

private:
    QMap<QString, CacheItem*> items;
};
