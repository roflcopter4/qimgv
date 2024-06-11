#pragma once

#include "components/cache/ThumbnailCache.h"
#include "LoaderRunnable.h"
#include <QThreadPool>

class Loader : public QObject
{
    Q_OBJECT

  public:
    explicit Loader(QObject *parent);
    ~Loader() override;

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

  Q_SIGNALS:
    void loadFinished(QSharedPointer<Image>, QString const &path);
    void loadFailed(QString const &path);

  private Q_SLOTS:
    void onLoadFinished(QSharedPointer<Image> const &, QString const &);
};
