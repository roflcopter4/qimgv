#pragma once

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>
#include <QtGlobal>

#ifdef Q_OS_WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include "Windows.h"
#endif

enum class FileOpResult {
    SUCCESS,
    DESTINATION_FILE_EXISTS,
    DESTINATION_DIR_EXISTS,
    SOURCE_NOT_WRITABLE,
    DESTINATION_NOT_WRITABLE,
    SOURCE_DOES_NOT_EXIST,
    DESTINATION_DOES_NOT_EXIST,
    DIRECTORY_NOT_EMPTY,
    NOTHING_TO_DO, // todo: maybe just return SUCCESS?
    OTHER_ERROR
};

class FileOperations
{
  public:
    static void copyFileTo(const QString &srcFilePath, const QString &destDirPath, bool force, FileOpResult &result);
    static void moveFileTo(const QString &srcFilePath, const QString &destDirPath, bool force, FileOpResult &result);
    static void rename(const QString &srcFilePath, const QString &newName, bool force, FileOpResult &result);
    static void removeFile(const QString &filePath, FileOpResult &result);
    static void removeDir(const QString &dirPath, bool recursive, FileOpResult &result);
    static void moveToTrash(const QString &filePath, FileOpResult &result);

    static QString decodeResult(const FileOpResult &result);

  private:
    static bool    moveToTrashImpl(const QString &filePath);
    static QString generateHash(const QString &str);
};
