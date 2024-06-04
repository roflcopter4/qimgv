#pragma once

#include <QSemaphore>
#include "sourcecontainers/image.h"

class CacheItem
{
  public:
    CacheItem();
    explicit CacheItem(std::shared_ptr<Image> const &contents);
    ~CacheItem();

    std::shared_ptr<Image> getContents();

    void lock();
    void unlock();
    ND int lockStatus() const;

    DELETE_COPY_MOVE_CONSTRUCTORS(CacheItem);

  private:
    std::shared_ptr<Image> contents;
    QSemaphore             *sem{};
};
