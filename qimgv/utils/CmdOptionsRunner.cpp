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
    QString out = u"Directory: " + dirPath + "\n"
                  u"File count: " + QString::number(list.size()) + "\n"
                  u"Size limit:" + QString::number(size) + u'x' + QString::number(size) + u"px\n"
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
    features << QS("USE_MPV");
#endif
#ifdef USE_EXIV2
    features << QS("USE_EXIV2");
#endif
#ifdef USE_KDE_BLUR
    features << QS("USE_KDE_BLUR");
#endif
#ifdef USE_OPENCV
    features << QS("USE_OPENCV");
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
