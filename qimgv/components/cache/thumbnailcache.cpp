#include "thumbnailcache.h"

ThumbnailCache::ThumbnailCache() {
    cacheDirPath = settings->thumbnailCacheDir();
}

QString ThumbnailCache::thumbnailPath(QString const &id) {
    return QString(cacheDirPath + id + QS(".png"));
}

bool ThumbnailCache::exists(QString const &id) {
    QString filePath = thumbnailPath(id);
    QFileInfo file(filePath);
    return file.exists() && file.isReadable();
}

void ThumbnailCache::saveThumbnail(QImage *image, QString const &id) {
    if(image) {
        QString filePath = thumbnailPath(id);
        image->save(filePath, "PNG", 15);
    }
}

QImage *ThumbnailCache::readThumbnail(QString const &id) {
    QString filePath = thumbnailPath(id);
    QFileInfo file(filePath);
    if(file.exists() && file.isReadable()) {
        QImage *thumb = new QImage();
        if(thumb->load(filePath)) {
            return thumb;
        } else {
            delete thumb;
            return nullptr;
        }
    } else {
        return nullptr;
    }
}
