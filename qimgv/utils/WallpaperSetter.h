#pragma once

#include <QString>
#include <QProcess>
#include <QDebug>
#ifdef Q_OS_WIN32
# include <Windows.h>
#endif
#include "utils/stuff.h"

class WallpaperSetter
{
public:
    WallpaperSetter();

    static void setWallpaper(QString const &path);
};
