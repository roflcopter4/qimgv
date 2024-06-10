#pragma once

#include "DirectoryWatcher.h"
#include "DirectoryWatcherEvent.h"
#include "DirectoryWatcherWorker.h"

#include <QStringList>
#include <QThread>
#include <QTimerEvent>
#include <QVariant>
#include <QtDebug>
#include <memory>

class DirectoryWatcherPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DirectoryWatcher)

  public:
    explicit DirectoryWatcherPrivate(DirectoryWatcher *qq, DirectoryWatcherWorker *w);
    ~DirectoryWatcherPrivate() override;

    virtual void setWatchPath(QString path);

  protected:
    DirectoryWatcher *q_ptr;
    QString           currentDirectory;
    QVector<QSharedPointer<WatcherEvent>>   directoryEvents;
    std::unique_ptr<DirectoryWatcherWorker> worker;
    std::unique_ptr<QThread>                workerThread;
};
