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

    ND qsizetype indexOfWatcherEvent(uint32_t cookie) const;
    ND qsizetype indexOfWatcherEvent(QString const &name) const;

    void handleModifyEvent(QString const &name);
    void handleDeleteEvent(QString const &name);
    void handleCreateEvent(QString const &name);
    void handleMovedFromEvent(QString const &name, uint32_t cookie);
    void handleMovedToEvent(QString const &name, uint32_t cookie);

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
