#pragma once

#include <qmetatype.h>
#include <QObject>

class DirectoryWatcherPrivate;

class DirectoryWatcher : public QObject
{
    Q_OBJECT

  public:
    static DirectoryWatcher *newInstance();
    ~DirectoryWatcher() override;

    virtual void    setWatchPath(QString path);
    ND virtual auto watchPath() const -> QString;
    ND bool         isObserving() const;

    DELETE_COPY_MOVE_CONSTRUCTORS(DirectoryWatcher);

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
    DirectoryWatcher(DirectoryWatcherPrivate *ptr);
    DirectoryWatcherPrivate *d_ptr;

  private:
    Q_DECLARE_PRIVATE(DirectoryWatcher)
};
