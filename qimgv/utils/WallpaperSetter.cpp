#include "WallpaperSetter.h"

WallpaperSetter::WallpaperSetter() = default;

void WallpaperSetter::setWallpaper(QString const &path)
{
#ifdef Q_OS_WIN32
    bool ok = SystemParametersInfoW(
        SPI_SETDESKWALLPAPER, 0,
        const_cast<LPVOID>(static_cast<LPCVOID>(path.utf16())),
        SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
    qDebug() << u"Wallpaper changed:" << (ok ? u"true" : u"false");
#elif defined __linux__
    QString command;
    command =
        u"qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript \'var allDesktops = desktops(); print "
        u"(allDesktops); for (i = 0; i < allDesktops.length; i++) { d = allDesktops[i]; d.wallpaperPlugin = \"org.kde.image\"; "
        u"d.currentConfigGroup = Array(\"Wallpaper\", \"org.kde.image\", \"General\"); d.writeConfig(\"Image\", \"" +
        path +
        u"\") } \'";
    QProcess process;
    //process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start(QS("sh"), QStringList() << QS("-c") << command);
    process.waitForFinished();
    process.close();

    qDebug() << u"In case that didnt work your cropped wallpaper is saved at:" << path;
#endif
}
