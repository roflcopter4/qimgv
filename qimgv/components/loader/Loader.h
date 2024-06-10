#pragma once

#include "components/cache/thumbnailcache.h"
#include "loaderrunnable.h"
#include <QThreadPool>

class Loader : public QObject
{
    Q_OBJECT

  public:
    explicit Loader();

    static QSharedPointer<Image> load(QString const &path);

    void loadAsyncPriority(QString const &path);
    void loadAsync(QString const &path);

    void clearTasks();
    bool isBusy() const;
    bool isLoading(QString const &path) const;

  private:
    QHash<QString, LoaderRunnable *> tasks;
    QThreadPool                     *pool;

    void clearPool();
    void doLoadAsync(QString const &path, int priority);

  signals:
    void loadFinished(QSharedPointer<Image>, const QString &path);
    void loadFailed(const QString &path);

  private slots:
    void onLoadFinished(QSharedPointer<Image> const &, const QString &);
};
