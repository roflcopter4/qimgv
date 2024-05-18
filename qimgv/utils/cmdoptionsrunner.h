#pragma once

#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QString>
#include "core.h"

class CmdOptionsRunner : public QObject {
    Q_OBJECT
public slots:
    void generateThumbs(QString const &dirPath, int size);
    void showBuildOptions();
};
