#pragma once

#include "../linux/LinuxDirectoryWatcher.h"
#include "../DirectoryWatcher_p.h"

#include <errno.h>
#include <QDebug>
#include <QTimer>

class LinuxFsEvent;

class LinuxDirectoryWatcherPrivate : public DirectoryWatcherPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(LinuxDirectoryWatcher)

  public:
    explicit LinuxDirectoryWatcherPrivate(LinuxDirectoryWatcher *qq);
    ~LinuxDirectoryWatcherPrivate() override;

    ND int indexOfWatcherEvent(uint cookie) const;
    ND int indexOfWatcherEvent(const QString &name) const;

    void handleModifyEvent(const QString &name);
    void handleDeleteEvent(const QString &name);
    void handleCreateEvent(const QString &name);
    void handleMovedFromEvent(const QString &name, uint cookie);
    void handleMovedToEvent(const QString &name, uint cookie);

  protected:
    void timerEvent(QTimerEvent *timerEvent) override;

  private Q_SLOTS:
    void dispatchFilesystemEvent(LinuxFsEvent *e);

  private:
    void setWatchPath(QString path) override;

    int watcher;
    int watchObject;
    QVector<QSharedPointer<WatcherEvent>> watcherEvents;
};
