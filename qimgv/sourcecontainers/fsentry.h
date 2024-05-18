#pragma once
#include <QString>
#include <filesystem>
#include "utils/stuff.h"

#if 0
class FSEntry
{
  public:
    FSEntry();
    explicit FSEntry(QString const &filePath);

    FSEntry(QString const &_path, QString const &_name, std::uintmax_t _size, std::filesystem::file_time_type _modifyTime, bool _isDirectory);
    FSEntry(QString const &_path, QString const &_name, std::uintmax_t _size, bool _isDirectory);
    FSEntry(QString const &_path, QString const &_name, bool _isDirectory);

    bool operator==(QString const &anotherPath) const;

    QString        path;
    QString        name;
    std::uintmax_t size;
    std::filesystem::file_time_type modifyTime;
    bool           isDirectory;
};
#endif

struct FSEntry
{
    FSEntry() = default;
    explicit FSEntry(QString const &filePath);

    FSEntry(QString   path,
            QString   name,
            uintmax_t size = 0, 
            std::filesystem::file_time_type modifyTime  = std::filesystem::file_time_type::min(), 
            bool      isDirectory = false);

    bool operator==(QString const &anotherPath) const;

    QString   path;
    QString   name;
    uintmax_t size = 0;
    std::filesystem::file_time_type modifyTime = std::filesystem::file_time_type::min();
    bool      isDirectory = false;
};
