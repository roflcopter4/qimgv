#pragma once

#include <QObject>
#include <QRunnable>
#include "utils/imagefactory.h"

class LoaderRunnable: public QObject, public QRunnable
{
    Q_OBJECT
public:
    LoaderRunnable(QString const &_path);
    void run() override;
private:
    QString path;
signals:
    void finished(std::shared_ptr<Image>, QString);
    void failed(QString);
};
