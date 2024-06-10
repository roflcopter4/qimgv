#pragma once

#include "utils/ImageFactory.h"
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

  Q_SIGNALS:
    void finished(QSharedPointer<Image>, QString);
    void failed(QString);
};
