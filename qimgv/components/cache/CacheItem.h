#pragma once

#include <QSemaphore>
#include "sourcecontainers/Image.h"

class CacheItem
{
  public:
    CacheItem();
    explicit CacheItem(QSharedPointer<Image> const &contents);
    ~CacheItem();
    DELETE_COPY_MOVE_ROUTINES(CacheItem);

    QSharedPointer<Image> getContents();

    void lock();
    void unlock();
    ND int lockStatus() const;

  private:
    QSharedPointer<Image> contents;
    QSemaphore             *sem{};
};
