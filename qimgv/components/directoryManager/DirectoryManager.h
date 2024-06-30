#pragma once

#include "Settings.h"
#include "gui/folderView/FileSystemModelCustom.h"
#include "sourcecontainers/FsEntry.h"
#include "utils/Stuff.h"
#include "watchers/DirectoryWatcher.h"
#include <QCollator>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QObject>
#include <QRegularExpression>
#include <QSize>
#include <QString>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

enum class FileListSource : uint8_t { // rename? wip
    INVALID,
    DIRECTORY,
    DIRECTORY_RECURSIVE,
    LIST,
};

// TODO: rename? EntrySomething?

class DirectoryManager final : public QObject
{
    Q_OBJECT

    using CompareFunction = bool (DirectoryManager::*)(FSEntry const &e1, FSEntry const &e2) const;

  public:
    explicit DirectoryManager(QObject *parent = nullptr);

    // ignored if the same dir is already opened
    bool setDirectory(QString const &);
    bool setDirectoryRecursive(QString const &);
    void sortEntryLists();
    void setSortingMode(SortingMode mode);

    bool insertFileEntry(QString const &filePath);
    bool forceInsertFileEntry(QString const &filePath);
    void removeFileEntry(QString const &filePath);
    void updateFileEntry(QString const &filePath);
    void renameFileEntry(QString const &oldFilePath, QString const &newFileName);

    bool insertDirEntry(QString const &dirPath);
    void removeDirEntry(QString const &dirPath);
    void renameDirEntry(QString const &oldDirPath, QString const &newDirName);
    // bool forceInsertDirEntry(const QString &dirPath);
    // void updateDirEntry(const QString &dirPath);

    ND QDateTime lastModified(QString const &filePath) const;
    ND QString   dirNameAt(qsizetype index) const;
    ND QString   dirPathAt(qsizetype index) const;
    ND QString   directoryPath() const;
    ND QString   fileNameAt(qsizetype index) const;
    ND QString   filePathAt(qsizetype index) const;
    ND QString   firstFile() const;
    ND QString   lastFile() const;
    ND QString   nextOfDir(QString const &dirPath) const;
    ND QString   nextOfFile(QString const &filePath) const;
    ND QString   prevOfDir(QString const &dirPath) const;
    ND QString   prevOfFile(QString const &filePath) const;
    ND auto      fileEntryAt(qsizetype index) const -> FSEntry const &;
    ND auto      fileList() const -> QStringList;
    ND auto      sortingMode() const -> SortingMode;
    ND auto      source() const -> FileListSource;
    ND bool      containsDir(QString const &dirPath) const;
    ND bool      containsFile(QString const &filePath) const;
    ND bool      fileWatcherActive() const;
    ND bool      isEmpty() const;
    ND bool      isSupportedFile(QString const &filePath) const;
    ND qsizetype dirCount() const;
    ND qsizetype fileCount() const;
    ND qsizetype indexOfDir(QString const &dirPath) const;
    ND qsizetype indexOfFile(QString const &filePath) const;
    ND qsizetype totalCount() const;

    ND static bool isDir(QString const &filePath);
    ND static bool isFile(QString const &filePath);

  private:
    ND bool path_entry_compare        (FSEntry const &e1, FSEntry const &e2) const;
    ND bool path_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;
    ND bool name_entry_compare        (FSEntry const &e1, FSEntry const &e2) const;
    ND bool name_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;
    ND bool date_entry_compare        (FSEntry const &e1, FSEntry const &e2) const;
    ND bool date_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;
    ND bool size_entry_compare        (FSEntry const &e1, FSEntry const &e2) const;
    ND bool size_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;
    ND bool checkFileRange(qsizetype index) const;
    ND bool checkDirRange(qsizetype index) const;
    ND auto compareFunction() const -> CompareFunction;

    void readSettings();
    void loadEntryList(QString const &directoryPath, bool recursive);
    void startFileWatcher(QString const &directoryPath);
    void stopFileWatcher();
    void addEntriesFromDirectory(std::vector<FSEntry> &entryVec, QString const &directoryPath);
    void addEntriesFromDirectoryRecursive(std::vector<FSEntry> &entryVec, QString const &directoryPath) const;

  private Q_SLOTS:
    void onFileAddedExternal(QString const &fileName);
    void onFileRemovedExternal(QString const &fileName);
    void onFileModifiedExternal(QString const &fileName);
    void onFileRenamedExternal(QString const &oldFileName, QString const &newFileName);

  Q_SIGNALS:
    void loaded(QString const &path);
    void sortingChanged();
    void fileRemoved(QString filePath, qsizetype);
    void fileModified(QString filePath);
    void fileAdded(QString filePath);
    void fileRenamed(QString fromPath, qsizetype indexFrom, QString toPath, qsizetype indexTo);
    void dirRemoved(QString dirPath, qsizetype);
    void dirAdded(QString dirPath);
    void dirRenamed(QString fromPath, qsizetype indexFrom, QString toPath, qsizetype indexTo);

  private:
    DirectoryWatcher    *watcher;
    std::vector<FSEntry> fileEntryVec;
    std::vector<FSEntry> dirEntryVec;
    QRegularExpression   regex;
    QCollator            collator;
    QString              mDirectoryPath;
    FileListSource       mListSource;
    SortingMode          mSortingMode;

    static FSEntry const defaultEntry;
};
