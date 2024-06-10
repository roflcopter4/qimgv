#pragma once

#include <QRunnable>
#include <QProcess>
#include <QThread>
#include <QCryptographicHash>
#include <ctime>
#include "sourcecontainers/Thumbnail.h"
#include "components/cache/ThumbnailCache.h"
#include "utils/ImageFactory.h"
#include "utils/ImageLib.h"
#include "Settings.h"
#include <memory>
#include <QImageWriter>

class ThumbnailerRunnable : public QObject, public QRunnable
{
    Q_OBJECT

  public:
    ThumbnailerRunnable(ThumbnailCache *cache, QString path, int size, bool crop, bool force);
    ~ThumbnailerRunnable() override;

    void run() override;

    static QSharedPointer<Thumbnail> generate(ThumbnailCache *cache, QString const &path, int size, bool crop, bool force);

  private:
    static auto generateIdString(QString const &path, int size, bool crop) -> QString;
    static auto createThumbnail(QString const &path, const char *format, int size, bool crop) -> std::pair<QImage *, QSize>;
    static auto createVideoThumbnail(QString const &path, int size, bool crop) -> std::pair<QImage *, QSize>;

    ThumbnailCache *cache = nullptr;
    QString path;
    int     size;
    bool    crop;
    bool    force;

  Q_SIGNALS:
    void taskStart(QString, int);
    void taskEnd(QSharedPointer<Thumbnail>, QString);
};
