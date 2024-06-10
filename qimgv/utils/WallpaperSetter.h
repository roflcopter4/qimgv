#pragma once

#include <QString>
#include <QProcess>
#include <QDebug>
#ifdef Q_OS_WIN32
# include <Windows.h>
#endif
#include "utils/Stuff.h"

class WallpaperSetter
{
public:
    WallpaperSetter();

    static void setWallpaper(QString const &path);
};
