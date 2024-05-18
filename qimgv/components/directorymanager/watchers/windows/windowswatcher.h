#ifndef WINDOWSWATCHER_H
#define WINDOWSWATCHER_H

#include "../directorywatcher.h"

class WindowsWatcherPrivate;

class WindowsWatcher final : public DirectoryWatcher
{
    Q_OBJECT

  public:
    explicit WindowsWatcher();
    explicit WindowsWatcher(QString const &path);
    ~WindowsWatcher() override = default;

    void setWatchPath(QString const &path) override;

  private:
    Q_DECLARE_PRIVATE(WindowsWatcher)
};

#endif // WINDOWSWATCHER_H
