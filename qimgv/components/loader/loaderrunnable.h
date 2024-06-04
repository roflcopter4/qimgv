#pragma once

#include "utils/imagefactory.h"
#include <QObject>
#include <QRunnable>

class LoaderRunnable : public QObject, public QRunnable
{
    Q_OBJECT

  public:
    explicit LoaderRunnable(QString path);

    void run() override;

  private:
    QString path_;

  signals:
    void finished(std::shared_ptr<Image>, QString);
    void failed(QString);
};
