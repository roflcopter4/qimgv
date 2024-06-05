#pragma once

#include <QThreadPool>
#include "components/thumbnailer/thumbnailerrunnable.h"
#include "components/cache/thumbnailcache.h"
#include "settings.h"

class Thumbnailer : public QObject
{
    Q_OBJECT

  public:
    explicit Thumbnailer();
    ~Thumbnailer() override;

    void clearTasks();
    void waitForDone();

    static QSharedPointer<Thumbnail> getThumbnail(QString const &filePath, int size);

  public Q_SLOTS:
    void getThumbnailAsync(QString const &path, int size, bool crop, bool force);

  private:
    ThumbnailCache         *cache;
    QThreadPool            *pool;
    QMultiMap<QString, int> runningTasks;

    void startThumbnailerThread(QString const &filePath, int size, bool crop, bool force);

  private Q_SLOTS:
    void onTaskStart(QString const &filePath, int size);
    void onTaskEnd(QSharedPointer<Thumbnail> const &thumbnail, QString const &filePath);

  Q_SIGNALS:
    void thumbnailReady(QSharedPointer<Thumbnail> thumbnail, QString filePath);
};
