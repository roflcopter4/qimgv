#include "fsentry.h"

FSEntry::FSEntry(QString const &filePath)
{
    auto    stdEntry = std::filesystem::directory_entry(util::QStringToStdString(filePath));
    QString fileName = util::StdStringToQString(stdEntry.path().filename().native());

    try {
        name        = fileName;
        path        = filePath;
        isDirectory = stdEntry.is_directory();

        if (!isDirectory) {
            size       = stdEntry.file_size();
            modifyTime = stdEntry.last_write_time();
        }
    } catch (std::filesystem::filesystem_error const &) {
    }
}

FSEntry::FSEntry(QString path, QString name, std::uintmax_t size, std::filesystem::file_time_type modifyTime, bool isDirectory)
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
    return this->path == anotherPath;
}
