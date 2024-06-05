#pragma once

#include <QSemaphore>
#include "sourcecontainers/image.h"

class CacheItem
{
  public:
    CacheItem();
    explicit CacheItem(QSharedPointer<Image> const &contents);
    ~CacheItem();

    QSharedPointer<Image> getContents();

    void lock();
    void unlock();
    ND int lockStatus() const;

    DELETE_COPY_MOVE_CONSTRUCTORS(CacheItem);

  private:
    QSharedPointer<Image> contents;
    QSemaphore             *sem{};
};
