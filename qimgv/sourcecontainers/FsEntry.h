#pragma once
#include <QString>
#include <filesystem>

struct FSEntry
{
  private:
    using ftime_t = std::filesystem::file_time_type;

  public:
    FSEntry() = default;
    explicit FSEntry(QString const &filePath);

    FSEntry(QString path,
            QString name,
            size_t  size        = 0, 
            ftime_t modifyTime  = std::filesystem::file_time_type::min(), 
            bool    isDirectory = false);

    bool operator==(FSEntry const &other) const { return path == other.path; }
    bool operator==(QString const &other) const { return path == other; }

    QString path;
    QString name;
    ftime_t modifyTime  = std::filesystem::file_time_type::min();
    size_t  size        = 0;
    bool    isDirectory = false;
};
