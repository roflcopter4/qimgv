#pragma once

#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QString>
#include "core.h"

class CmdOptionsRunner : public QObject
{
    Q_OBJECT

  public Q_SLOTS:
    static void generateThumbs(QString const &dirPath, int size);
    static void showBuildOptions();
};
