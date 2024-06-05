#pragma once

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

#include "settings.h"
#include "sourcecontainers/fsentry.h"
#include "utils/stuff.h"
#include "watchers/DirectoryWatcher.h"

enum class FileListSource { // rename? wip
    DIRECTORY,
    DIRECTORY_RECURSIVE,
    LIST
};

// TODO: rename? EntrySomething?

class DirectoryManager : public QObject
{
    Q_OBJECT

    using CompareFunction = bool (DirectoryManager::*)(FSEntry const &e1, FSEntry const &e2) const;

  public:
    DirectoryManager();
    // ignored if the same dir is already opened
    bool setDirectory(QString const &);
    bool setDirectoryRecursive(QString const &);
    void sortEntryLists();
    void setSortingMode(SortingMode mode);

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
    ND qsizetype indexOfDir(QString const &dirPath) const;
    ND qsizetype indexOfFile(QString const &filePath) const;
    ND bool      containsDir(QString const &dirPath) const;
    ND bool      containsFile(QString const &filePath) const;
    ND bool      fileWatcherActive() const;
    ND bool      isEmpty() const;
    ND bool      isSupportedFile(QString const &filePath) const;
    ND qsizetype dirCount() const;
    ND qsizetype fileCount() const;
    ND qsizetype totalCount() const;

    ND static bool isDir(QString const &filePath);
    ND static bool isFile(QString const &filePath);

    ND SortingMode    sortingMode() const;
    ND FSEntry const &fileEntryAt(qsizetype index) const;

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

    ND FileListSource source() const;
    ND QStringList    fileList() const;

  private:
    QRegularExpression   regex;
    QCollator            collator;
    std::vector<FSEntry> fileEntryVec;
    std::vector<FSEntry> dirEntryVec;
    FSEntry const        defaultEntry;
    QString              mDirectoryPath;
    DirectoryWatcher    *watcher;
    SortingMode          mSortingMode;
    FileListSource       mListSource;

    void readSettings();
    void loadEntryList(QString const &directoryPath, bool recursive);

    ND bool path_entry_compare(FSEntry const &e1, FSEntry const &e2) const;
    ND bool path_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;
    ND bool name_entry_compare(FSEntry const &e1, FSEntry const &e2) const;
    ND bool name_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;
    ND bool date_entry_compare(FSEntry const &e1, FSEntry const &e2) const;
    ND bool date_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;
    ND bool size_entry_compare(FSEntry const &e1, FSEntry const &e2) const;
    ND bool size_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const;

    void startFileWatcher(QString const &directoryPath);
    void stopFileWatcher();
    void addEntriesFromDirectory(std::vector<FSEntry> &entryVec, QString const &directoryPath);
    void addEntriesFromDirectoryRecursive(std::vector<FSEntry> &entryVec, QString const &directoryPath) const;

    ND bool checkFileRange(qsizetype index) const;
    ND bool checkDirRange(qsizetype index) const;

    CompareFunction compareFunction();

  private slots:
    void onFileAddedExternal(QString const &fileName);
    void onFileRemovedExternal(QString const &fileName);
    void onFileModifiedExternal(QString const &fileName);
    void onFileRenamedExternal(QString const &oldFileName, QString const &newFileName);

  signals:
    void loaded(QString const &path);
    void sortingChanged();
    void fileRemoved(QString filePath, int);
    void fileModified(QString filePath);
    void fileAdded(QString filePath);
    void fileRenamed(QString fromPath, qsizetype indexFrom, QString toPath, qsizetype indexTo);

    void dirRemoved(QString dirPath, int);
    void dirAdded(QString dirPath);
    void dirRenamed(QString fromPath, qsizetype indexFrom, QString toPath, qsizetype indexTo);
};
