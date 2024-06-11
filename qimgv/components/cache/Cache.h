#pragma once

#include <QDebug>
#include <QMap>
#include <QSemaphore>
#include <QMutexLocker>
#include "sourcecontainers/Image.h"
#include "components/cache/CacheItem.h"
#include "utils/ImageFactory.h"

class Cache
{
  public:
    explicit Cache() = default;
    ~Cache();

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
