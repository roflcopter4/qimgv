#pragma once

#include <QObject>

class DirectoryWatcherWorker : public QObject
{
    Q_OBJECT

  public:
    DirectoryWatcherWorker();
    ~DirectoryWatcherWorker() override = default;
    DEFAULT_COPY_MOVE_ROUTINES(DirectoryWatcherWorker);

    virtual void run() = 0;

  Q_SIGNALS:
    void error(QString const &errorMessage);
    void started();
    void finished();

  public Q_SLOTS:
    void setRunning(bool running);

  protected:
    std::atomic_bool isRunning = false;
};
