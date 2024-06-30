#include "FsEntry.h"
#include "utils/Stuff.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

FSEntry::FSEntry(QString const &filePath)
{
    try {
        auto stdEntry = std::filesystem::directory_entry(util::QStringToStdPath(filePath));

        name = util::StdPathToQString(stdEntry.path().filename());
        path = QDir::isAbsolutePath(filePath) ? filePath
                                              : QDir::fromNativeSeparators(util::StdPathToQString(stdEntry));
        isDirectory = stdEntry.is_directory();
        if (!isDirectory) {
            size       = stdEntry.file_size();
            modifyTime = stdEntry.last_write_time();
        }
    } catch (std::filesystem::filesystem_error const &err) {
        qDebug() << u"Caught file system error:" << QString::fromUtf8(err.what());
    }
}

FSEntry::FSEntry(QString path, QString name, size_t size, ftime_t modifyTime, bool isDirectory)
    : path(std::move(path)),
      name(std::move(name)),
      modifyTime(modifyTime),
      size(size),
      isDirectory(isDirectory)
{}
