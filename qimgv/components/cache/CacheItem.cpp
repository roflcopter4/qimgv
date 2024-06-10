#include "CacheItem.h"

CacheItem::CacheItem()
    : sem(new QSemaphore(1))
{
}

CacheItem::CacheItem(QSharedPointer<Image> const &contents)
    : contents(contents),
      sem(new QSemaphore(1))
{
}

CacheItem::~CacheItem()
{
    delete sem;
}

QSharedPointer<Image> CacheItem::getContents()
{
    return contents;
}

void CacheItem::lock()
{
    sem->acquire(1);
}

void CacheItem::unlock()
{
    sem->release(1);
}

int CacheItem::lockStatus() const
{
    return sem->available();
}
