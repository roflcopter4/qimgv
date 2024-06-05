#include "cache.h"

bool Cache::contains(QString const &path) const
{
    return items.contains(path);
}

bool Cache::insert(QSharedPointer<Image> const &img)
{
    if (img) {
        if (items.contains(img->filePath())) {
            return false;
        } else {
            items.insert(img->filePath(), new CacheItem(img));
            return true;
        }
    }
    // TODO: what state returns here ??
    return true;
}

void Cache::remove(QString const &path)
{
    if (items.contains(path)) {
        items[path]->lock();
        auto *item = items.take(path);
        delete item;
    }
}

void Cache::clear()
{
    for (auto const &path : items.keys()) {
        items[path]->lock();
        CacheItem *item = items.take(path);
        delete item;
    }
}

QSharedPointer<Image> Cache::get(QString const &path) const
{
    if (items.contains(path)) {
        CacheItem *item = items.value(path);
        return item->getContents();
    }
    return nullptr;
}

bool Cache::reserve(QString const &path)
{
    if (items.contains(path)) {
        items[path]->lock();
        return true;
    }
    return false;
}

bool Cache::release(QString const &path)
{
    if (items.contains(path)) {
        items[path]->unlock();
        return true;
    }
    return false;
}

// removes all items except the ones in list
void Cache::trimTo(QStringList const &list)
{
    for (auto const &path : items.keys()) {
        if (!list.contains(path)) {
            items[path]->lock();
            CacheItem *item = items.take(path);
            delete item;
        }
    }
}

QList<QString> Cache::keys() const
{
    return items.keys();
}
