#pragma once

#include <QDebug>
#include <QMap>
#include <QSemaphore>
#include <QMutexLocker>
#include "sourcecontainers/image.h"
#include "components/cache/cacheitem.h"
#include "utils/imagefactory.h"

class Cache
{
  public:
    explicit Cache() = default;

    void remove(QString const &path);
    bool insert(QSharedPointer<Image> const &img);
    void trimTo(QStringList const &list);
    bool release(QString const &path);
    bool reserve(QString const &path);
    void clear();

    ND auto get(QString const &path) const -> QSharedPointer<Image>;
    ND auto keys() const -> QList<QString>;
    ND bool contains(QString const &path) const;

  private:
    QMap<QString, CacheItem *> items;
};
