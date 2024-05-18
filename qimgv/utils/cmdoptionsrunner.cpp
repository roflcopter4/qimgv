#include "cmdoptionsrunner.h"

void CmdOptionsRunner::generateThumbs(QString const &dirPath, int size) {
    if(size <= 50 || size > 400) {
        qDebug() << QSV("Error: Invalid thumbnail size.");
        qDebug() << QSV("Please specify a value between [50, 400].");
        qDebug() << QSV("Example:  qimgv --gen-thumbs=/home/user/Pictures/ --gen-thumbs-size=120");
        QCoreApplication::exit(1);
        return;
    }

    Thumbnailer th;
    DirectoryManager dm;
    if(!dm.setDirectoryRecursive(dirPath)) {
        qDebug() << QSV("Error: Invalid path.");
        QCoreApplication::exit(1);
        return;
    }

    auto list = dm.fileList();

    qDebug() << QSV("\nDirectory:") << dirPath;
    qDebug() << QSV("File count:") << list.size();
    qDebug() << QSV("Size limit:") << size << QSV("x") << size << QSV("px");
    qDebug() << QSV("Generating thumbnails...");

    for(auto path : list)
        th.getThumbnailAsync(path, size, false, false);

    th.waitForDone();
    qDebug() << QSV("\nDone.");
    QCoreApplication::quit();
}

void CmdOptionsRunner::showBuildOptions() {
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
    qDebug() << QSV("\nEnabled build options:");
    if(!features.count())
        qDebug() << QSV("   --");
    for(int i = 0; i < features.count(); i++)
        qDebug() << QSV("   ") << features.at(i);
    QCoreApplication::quit();
}
