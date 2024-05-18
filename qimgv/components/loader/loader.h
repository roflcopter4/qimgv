#pragma once

#include <QThreadPool>
#include "components/cache/thumbnailcache.h"
#include "loaderrunnable.h"

class Loader : public QObject {
    Q_OBJECT
public:
    explicit                      Loader();
    static std::shared_ptr<Image> load(QString const &path);
    void                          loadAsyncPriority(QString const &path);
    void                          loadAsync(QString const &path);

    void clearTasks();
    bool isBusy() const;
    bool isLoading(QString const &path) const;
private:
    QHash<QString, LoaderRunnable*> tasks;
    QThreadPool *                   pool;    
    void                            clearPool();
    void                            doLoadAsync(QString const &path, int priority);

signals:
    void loadFinished(std::shared_ptr<Image>, const QString &path);
    void loadFailed(const QString &path);

private slots:
    void onLoadFinished(std::shared_ptr<Image> const &, const QString&);
};
