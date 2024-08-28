#include "CmdOptionsRunner.h"

#ifdef Q_OS_WIN32
# define Win32OpenConsole() util::OpenConsoleWindow()
# define Win32WaitForKey()  util::WaitForAnyKey()
#else
# define Win32OpenConsole() ((void)0)
# define Win32WaitForKey()  ((void)0)
#endif

void CmdOptionsRunner::generateThumbs(QString const &dirPath, int size)
{
    Win32OpenConsole();

    if (size <= 50 || size > 400) {
        std::cout << reinterpret_cast<char const *>(
            u8"Error: Invalid thumbnail size.\n"
            u8"Please specify a value between [50, 400].\n"
            u8"Example: qimgv --gen-thumbs=/home/user/Pictures/ --gen-thumbs-size=120\n");
        Win32WaitForKey();
        QCoreApplication::exit(1);
        return;
    }

    auto dm = std::make_unique<DirectoryManager>();
    if (!dm->setDirectoryRecursive(dirPath)) {
        QString out = u"Error: Invalid path \"" + dirPath + u"\".\n";
        std::cout << out.toStdString();
        Win32WaitForKey();
        QCoreApplication::exit(1);
        return;
    }

    auto list = dm->fileList();
    QString out = u"Directory: " + dirPath + u"\n"
                  u"File count: " + QString::number(list.size()) + u"\n"
                  u"Size limit: " + QString::number(size) + u'x' + QString::number(size) + u"px\n"
                  u"Generating thumbnails...\n";
    std::cout << out.toStdString();

    Thumbnailer th(nullptr);
    for (auto const &path : list)
        th.getThumbnailAsync(path, size, false, false);

    th.waitForDone();
    std::cout << reinterpret_cast<char const *>(u8"Done\n");
    Win32WaitForKey();
    QCoreApplication::quit();
}

void CmdOptionsRunner::showBuildOptions()
{
    Win32OpenConsole();

    QStringList features;
#ifdef USE_MPV
    features << u"USE_MPV"_s;
#endif
#ifdef USE_EXIV2
    features << u"USE_EXIV2"_s;
#endif
#ifdef USE_KDE_BLUR
    features << u"USE_KDE_BLUR"_s;
#endif
#ifdef USE_OPENCV
    features << u"USE_OPENCV"_s;
#endif

    QString out;
    out += u"Enabled build options:"_sv;
    if (!features.isEmpty()) {
        for (auto const &s : features)
            out += u"   " + s;
    } else {
        out += u"   --"_sv;
    }
    std::cout << out.toStdString() << '\n';

    Win32WaitForKey();
    QCoreApplication::quit();
}
