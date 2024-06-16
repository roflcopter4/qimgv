#include "FsEntry.h"
#include <QDebug>

FSEntry::FSEntry(QString const &filePath)
{
    auto    stdEntry = std::filesystem::directory_entry(util::QStringToStdPath(filePath));
    QString fileName = util::StdPathToQString(stdEntry.path().filename());

    try {
        name        = std::move(fileName);
        path        = filePath;
        isDirectory = stdEntry.is_directory();

        if (!isDirectory) {
            size       = stdEntry.file_size();
            modifyTime = stdEntry.last_write_time();
        }
    } catch (std::filesystem::filesystem_error const &err) {
        qDebug() << u"Caught file system error:" << QString::fromUtf8(err.what());
    }
}

FSEntry::FSEntry(QString path, QString name, size_t size, std::filesystem::file_time_type modifyTime, bool isDirectory)
    : path(std::move(path)),
      name(std::move(name)),
      size(size),
      modifyTime(modifyTime),
      isDirectory(isDirectory)
{}

#if 0
FSEntry::FSEntry( QString const &_path, QString const &_name, std::uintmax_t _size, bool _isDirectory)
    : path(_path),
      name(_name),
      size(_size),
      isDirectory(_isDirectory)
{}

FSEntry::FSEntry( QString const &_path, QString const &_name, bool _isDirectory)
    : path(_path),
      name(_name),
      isDirectory(_isDirectory)
{}
#endif

bool FSEntry::operator==(QString const &anotherPath) const
{
    return path == anotherPath;
}
