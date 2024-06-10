#pragma once

#include <qmetatype.h>
#include <QObject>

class DirectoryManager;
class DirectoryWatcherPrivate;

class DirectoryWatcher : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DirectoryWatcher)

  public:
    static DirectoryWatcher *newInstance(DirectoryManager *parent);
    ~DirectoryWatcher() override;

    void    setWatchPath(QString const &path);
    ND auto watchPath() const -> QString;
    ND bool isObserving() const;

  public Q_SLOTS:
    void observe();
    void stopObserving();

  Q_SIGNALS:
    void fileCreated(QString const &filePath);
    void fileDeleted(QString const &filePath);
    void fileRenamed(QString const &old, QString const &now);
    void fileModified(QString const &filePath);

    void observingStarted();
    void observingStopped();

  protected:
    explicit DirectoryWatcher(DirectoryWatcherPrivate *ptr, DirectoryManager *parent);

    std::unique_ptr<DirectoryWatcherPrivate> d_ptr;
};
