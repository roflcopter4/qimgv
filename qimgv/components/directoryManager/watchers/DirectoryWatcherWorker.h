#pragma once

#include <QObject>

class DirectoryWatcherWorker : public QObject
{
    Q_OBJECT

  public:
    DirectoryWatcherWorker();
    ~DirectoryWatcherWorker() override = default;
    virtual void run() = 0;

  public Q_SLOTS:
    bool setRunning(bool running);

  Q_SIGNALS:
    void error(QString const &errorMessage);
    void started();
    void finished();

  protected:
    std::atomic_bool isRunning = false;
};
