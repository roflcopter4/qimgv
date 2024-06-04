#include "thumbnailcache.h"

ThumbnailCache::ThumbnailCache()
    : cacheDirPath(settings->thumbnailCacheDir())
{}

QString ThumbnailCache::thumbnailPath(QString const &id) const
{
    return QString(cacheDirPath + id + QSV(".png"));
}

static bool checkExists(QString const &filePath)
{
    QFileInfo file(filePath);
    return file.exists() && file.isReadable();
}

bool ThumbnailCache::exists(QString const &id) const
{
    return checkExists(thumbnailPath(id));
}

void ThumbnailCache::saveThumbnail(QImage const *image, QString const &id) const
{
    if (image) {
        QString filePath = thumbnailPath(id);
        image->save(filePath, "PNG", 15);
    }
}

QImage *ThumbnailCache::readThumbnail(QString const &id) const
{
    QString filePath = thumbnailPath(id);
    if (checkExists(filePath)) {
        auto *thumb = new QImage();
        if (thumb->load(filePath))
            return thumb;
        delete thumb;
    }
    return nullptr;
}
