#include "DirectoryManager.h"
#include "utils/Stuff.h"
#include <ranges>

#include <QMessageBox>

using util::pathsep;

DirectoryManager::DirectoryManager(QObject *parent)
    : QObject(parent),
      watcher(nullptr),
      mSortingMode(SortingMode::NAME),
      mListSource(FileListSource::INVALID)
{
    regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    collator.setNumericMode(true);

    readSettings();
    setSortingMode(settings->sortingMode());
    connect(settings, &Settings::settingsChanged, this, &DirectoryManager::readSettings);
}

template <typename Ty, typename Pred>
typename std::vector<Ty>::iterator
insert_sorted(std::vector<Ty> &vec, Ty const &item, Pred pred)
{
    return vec.insert(std::upper_bound(vec.begin(), vec.end(), item, pred), item);
}

bool DirectoryManager::path_entry_compare(FSEntry const &e1, FSEntry const &e2) const
{
    return collator.compare(e1.path, e2.path) < 0;
};

bool DirectoryManager::path_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const
{
    return collator.compare(e1.path, e2.path) > 0;
};

bool DirectoryManager::name_entry_compare(FSEntry const &e1, FSEntry const &e2) const
{
    return collator.compare(e1.name, e2.name) < 0;
};

bool DirectoryManager::name_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const
{
    return collator.compare(e1.name, e2.name) > 0;
};

// ReSharper disable once CppMemberFunctionMayBeStatic
bool DirectoryManager::date_entry_compare(FSEntry const &e1, FSEntry const &e2) const
{
    return e1.modifyTime < e2.modifyTime;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool DirectoryManager::date_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const
{
    return e1.modifyTime > e2.modifyTime;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool DirectoryManager::size_entry_compare(FSEntry const &e1, FSEntry const &e2) const
{
    return e1.size < e2.size;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool DirectoryManager::size_entry_compare_reverse(FSEntry const &e1, FSEntry const &e2) const
{
    return e1.size > e2.size;
}

DirectoryManager::CompareFunction DirectoryManager::compareFunction()
{
    CompareFunction cmpFn;

    switch (mSortingMode) { // NOLINT(clang-diagnostic-switch-enum)
    default:
    case SortingMode::NAME:      cmpFn = &DirectoryManager::path_entry_compare;         break;
    case SortingMode::NAME_DESC: cmpFn = &DirectoryManager::path_entry_compare_reverse; break;
    case SortingMode::SIZE:      cmpFn = &DirectoryManager::size_entry_compare;         break;
    case SortingMode::SIZE_DESC: cmpFn = &DirectoryManager::size_entry_compare_reverse; break;
    case SortingMode::TIME:      cmpFn = &DirectoryManager::date_entry_compare;         break;
    case SortingMode::TIME_DESC: cmpFn = &DirectoryManager::date_entry_compare_reverse; break;
    }

    return cmpFn;
}

void DirectoryManager::startFileWatcher(QString const &directoryPath)
{
    if (directoryPath.isEmpty())
        return;
    if (!watcher)
        watcher = DirectoryWatcher::newInstance(this);
    watcher->setParent(this);

    connect(watcher, &DirectoryWatcher::fileCreated,  this, &DirectoryManager::onFileAddedExternal,    Qt::UniqueConnection);
    connect(watcher, &DirectoryWatcher::fileDeleted,  this, &DirectoryManager::onFileRemovedExternal,  Qt::UniqueConnection);
    connect(watcher, &DirectoryWatcher::fileModified, this, &DirectoryManager::onFileModifiedExternal, Qt::UniqueConnection);
    connect(watcher, &DirectoryWatcher::fileRenamed,  this, &DirectoryManager::onFileRenamedExternal,  Qt::UniqueConnection);

    watcher->setWatchPath(directoryPath);
    watcher->observe();
}

void DirectoryManager::stopFileWatcher()
{
    if (!watcher)
        return;

    watcher->stopObserving();

    disconnect(watcher, &DirectoryWatcher::fileCreated,  this, &DirectoryManager::onFileAddedExternal);
    disconnect(watcher, &DirectoryWatcher::fileDeleted,  this, &DirectoryManager::onFileRemovedExternal);
    disconnect(watcher, &DirectoryWatcher::fileModified, this, &DirectoryManager::onFileModifiedExternal);
    disconnect(watcher, &DirectoryWatcher::fileRenamed,  this, &DirectoryManager::onFileRenamedExternal);
}

// ##############################################################
// ####################### PUBLIC METHODS #######################
// ##############################################################

void DirectoryManager::readSettings()
{
    regex.setPattern(settings->supportedFormatsRegex());
}

bool DirectoryManager::setDirectory(QString const &dirPath)
{
    if (dirPath.isEmpty())
        return false;
    auto path = QFileInfo(dirPath);
    if (!path.exists()) {
        qDebug() << u"[DirectoryManager] Error - path" << dirPath << u"does not exist.";
        return false;
    }
    if (!path.isDir()) {
        qDebug() << u"[DirectoryManager] Error - path is not a directory.";
        return false;
    }
    auto dir = QDir(dirPath);
    if (!dir.isReadable()) {
        qDebug() << u"[DirectoryManager] Error - cannot read directory.";
        return false;
    }
    mListSource    = FileListSource::DIRECTORY;
    mDirectoryPath = dirPath;

    loadEntryList(dirPath, false);
    sortEntryLists();
    emit loaded(dirPath);
    startFileWatcher(dirPath);
    return true;
}

bool DirectoryManager::setDirectoryRecursive(QString const &dirPath)
{
    if (dirPath.isEmpty())
        return false;
    auto path = QFileInfo(dirPath);
    if (!path.exists()) {
        qDebug() << u"[DirectoryManager] Error - path" << path << u"does not exist.";
        return false;
    }
    if (!path.isDir()) {
        qDebug() << u"[DirectoryManager] Error - path is not a directory.";
        return false;
    }
    stopFileWatcher();
    mListSource    = FileListSource::DIRECTORY_RECURSIVE;
    mDirectoryPath = dirPath;
    loadEntryList(dirPath, true);
    sortEntryLists();
    emit loaded(dirPath);
    return true;
}

QString DirectoryManager::directoryPath() const
{
    if (mListSource == FileListSource::DIRECTORY || mListSource == FileListSource::DIRECTORY_RECURSIVE)
        return mDirectoryPath;
    else
        return {};
}

qsizetype DirectoryManager::indexOfFile(QString const &filePath) const
{
#ifdef _WIN32
    if (filePath.contains(u'\\'))
        qCritical() << u"Found erroneous '\\' character in path " << filePath << u". This is bad.";
#endif
    auto item = std::ranges::find_if(fileEntryVec,
                                     [&filePath](FSEntry const &e) { return e.path == filePath; });
    if (item != fileEntryVec.end())
        return distance(fileEntryVec.begin(), item);
    return -1;
}

qsizetype DirectoryManager::indexOfDir(QString const &dirPath) const
{
#ifdef _WIN32
    if (dirPath.contains(u'\\'))
        qCritical() << u"Found erroneous '\\' character in path " << dirPath << u". This is bad.";
#endif
    auto item = std::ranges::find_if(dirEntryVec,
                                     [&dirPath](FSEntry const &e) { return e.path == dirPath; });
    if (item != dirEntryVec.end())
        return distance(dirEntryVec.begin(), item);
    return -1;
}

QString DirectoryManager::filePathAt(qsizetype index) const
{
    return checkFileRange(index) ? fileEntryVec[index].path : QString{};
}

QString DirectoryManager::fileNameAt(qsizetype index) const
{
    return checkFileRange(index) ? fileEntryVec[index].name : QString{};
}

QString DirectoryManager::dirPathAt(qsizetype index) const
{
    return checkDirRange(index) ? dirEntryVec[index].path : QString{};
}

QString DirectoryManager::dirNameAt(qsizetype index) const
{
    return checkDirRange(index) ? dirEntryVec[index].name : QString{};
}

QString DirectoryManager::firstFile() const
{
    QString filePath{};
    if (!fileEntryVec.empty())
        filePath = fileEntryVec.front().path;
    return filePath;
}

QString DirectoryManager::lastFile() const
{
    QString filePath{};
    if (!fileEntryVec.empty())
        filePath = fileEntryVec.back().path;
    return filePath;
}

QString DirectoryManager::prevOfFile(QString const &filePath) const
{
    QString   prevFilePath{};
    qsizetype currentIndex = indexOfFile(filePath);
    if (currentIndex > 0)
        prevFilePath = fileEntryVec[currentIndex - 1].path;
    return prevFilePath;
}

QString DirectoryManager::nextOfFile(QString const &filePath) const
{
    QString   nextFilePath{};
    qsizetype currentIndex = indexOfFile(filePath);
    if (currentIndex >= 0 && currentIndex < qsizetype(fileEntryVec.size() - 1))
        nextFilePath = fileEntryVec[currentIndex + 1].path;
    return nextFilePath;
}

QString DirectoryManager::prevOfDir(QString const &dirPath) const
{
    QString   prevDirectoryPath{};
    qsizetype currentIndex = indexOfDir(dirPath);
    if (currentIndex > 0)
        prevDirectoryPath = dirEntryVec[currentIndex - 1].path;
    return prevDirectoryPath;
}

QString DirectoryManager::nextOfDir(QString const &dirPath) const
{
    QString   nextDirectoryPath{};
    qsizetype currentIndex = indexOfDir(dirPath);
    if (currentIndex >= 0 && currentIndex < qsizetype(dirEntryVec.size() - 1))
        nextDirectoryPath = dirEntryVec[currentIndex + 1].path;
    return nextDirectoryPath;
}

bool DirectoryManager::checkFileRange(qsizetype index) const
{
    return index >= 0 && index < qsizetype(fileEntryVec.size());
}

bool DirectoryManager::checkDirRange(qsizetype index) const
{
    return index >= 0 && index < qsizetype(dirEntryVec.size());
}

qsizetype DirectoryManager::totalCount() const
{
    return fileCount() + dirCount();
}

qsizetype DirectoryManager::fileCount() const
{
    return fileEntryVec.size();
}

qsizetype DirectoryManager::dirCount() const
{
    return dirEntryVec.size();
}

FSEntry const &DirectoryManager::fileEntryAt(qsizetype index) const
{
    if (checkFileRange(index))
        return fileEntryVec[index];
    return defaultEntry;
}

QDateTime DirectoryManager::lastModified(QString const &filePath) const
{
    QFileInfo info;
    if (containsFile(filePath))
        info.setFile(filePath);
    return info.lastModified();
}

// TODO: what about symlinks?
bool DirectoryManager::isSupportedFile(QString const &filePath) const
{
    return (isFile(filePath) && regex.match(filePath).hasMatch());
}

bool DirectoryManager::isFile(QString const &filePath)
{
    auto path = util::QStringToStdPath(filePath);
    return exists(path) && is_regular_file(path);
}

bool DirectoryManager::isDir(QString const &filePath)
{
    auto path = util::QStringToStdPath(filePath);
    return exists(path) && is_directory(path);
}

bool DirectoryManager::isEmpty() const
{
    return fileEntryVec.empty();
}

bool DirectoryManager::containsFile(QString const &filePath) const
{
    return (std::find(fileEntryVec.begin(), fileEntryVec.end(), filePath) != fileEntryVec.end());
}

bool DirectoryManager::containsDir(QString const &dirPath) const
{
    return (std::find(dirEntryVec.begin(), dirEntryVec.end(), dirPath) != dirEntryVec.end());
}

// ##############################################################
// ###################### PRIVATE METHODS #######################
// ##############################################################
void DirectoryManager::loadEntryList(QString const &directoryPath, bool recursive)
{
    dirEntryVec.clear();
    fileEntryVec.clear();
    if (recursive) // load files only
        addEntriesFromDirectoryRecursive(fileEntryVec, directoryPath);
    else // load dirs & files
        addEntriesFromDirectory(fileEntryVec, directoryPath);
}

// both directories & files
void DirectoryManager::addEntriesFromDirectory(std::vector<FSEntry> &entryVec, QString const &directoryPath)
{
    QRegularExpressionMatch match;
    for (auto const &entry : std::filesystem::directory_iterator(util::QStringToStdPath(directoryPath)))
    {
        auto info  = QFileInfo(entry.path());
        auto name  = info.fileName();
#ifndef Q_OS_WIN32
        // ignore hidden files
        if (name.startsWith(QSV(".")))
            continue;
#endif
        auto path  = info.absoluteFilePath();

        match = regex.match(name);
        if (entry.is_directory()) { // this can still throw std::bad_alloc ..
            FSEntry newEntry;
            try {
                newEntry.name        = name;
                newEntry.path        = path;
                newEntry.isDirectory = true;
                // newEntry.size = entry.file_size();
                // newEntry.modifyTime = entry.last_write_time();
            } catch (std::filesystem::filesystem_error const &err) {
                qDebug() << u"[DirectoryManager]" << err.what();
                continue;
            }
            dirEntryVec.emplace_back(newEntry);
        } else if (match.hasMatch()) {
            FSEntry newEntry;
            try {
                newEntry.name        = name;
                newEntry.path        = path;
                newEntry.isDirectory = false;
                newEntry.size        = entry.file_size();
                newEntry.modifyTime  = entry.last_write_time();
            } catch (std::filesystem::filesystem_error const &err) {
                qDebug() << u"[DirectoryManager]" << err.what();
                continue;
            }
            entryVec.emplace_back(newEntry);
        }
    }
}

void DirectoryManager::addEntriesFromDirectoryRecursive(
    std::vector<FSEntry> &entryVec,
    QString const        &directoryPath) const
{
    for (auto const &entry : std::filesystem::recursive_directory_iterator(util::QStringToStdPath(directoryPath)))
    {
        auto info  = QFileInfo(entry.path());
        auto name  = info.fileName();
        auto path  = info.absoluteFilePath();
        auto match = regex.match(name);

        if (!entry.is_directory() && match.hasMatch()) {
            FSEntry newEntry;
            try {
                newEntry.name        = name;
                newEntry.path        = path;
                newEntry.isDirectory = false;
                newEntry.size        = entry.file_size();
                newEntry.modifyTime  = entry.last_write_time();
            } catch (std::filesystem::filesystem_error const &err) {
                qDebug() << u"[DirectoryManager]" << err.what();
                continue;
            }
            entryVec.emplace_back(newEntry);
        }
    }
}

void DirectoryManager::sortEntryLists()
{
    if (settings->sortFolders())
        std::ranges::sort(dirEntryVec, std::bind(compareFunction(), this, std::placeholders::_1, std::placeholders::_2));
    else
        std::ranges::sort(dirEntryVec, std::bind(&DirectoryManager::path_entry_compare, this, std::placeholders::_1, std::placeholders::_2));

    std::ranges::sort(fileEntryVec, std::bind(compareFunction(), this, std::placeholders::_1, std::placeholders::_2));
}

void DirectoryManager::setSortingMode(SortingMode mode)
{
    if (mode != mSortingMode) {
        mSortingMode = mode;
        if (fileEntryVec.size() > 1 || dirEntryVec.size() > 1) {
            sortEntryLists();
            emit sortingChanged();
        }
    }
}

SortingMode DirectoryManager::sortingMode() const
{
    return mSortingMode;
}

// Entry management

bool DirectoryManager::insertFileEntry(QString const &filePath)
{
    if (!isSupportedFile(filePath))
        return false;
    return forceInsertFileEntry(filePath);
}

// skips filename regex check
bool DirectoryManager::forceInsertFileEntry(QString const &filePath)
{
    if (!this->isFile(filePath) || containsFile(filePath))
        return false;

    auto    stdEntry = std::filesystem::directory_entry(util::QStringToStdPath(filePath));
    QString fileName = util::StdStringToQString(stdEntry.path().filename().native());
    auto    fsEntry  = FSEntry(filePath, fileName, stdEntry.file_size(), stdEntry.last_write_time(), stdEntry.is_directory());

    insert_sorted(fileEntryVec, fsEntry, std::bind(compareFunction(), this, std::placeholders::_1, std::placeholders::_2));
    if (!directoryPath().isEmpty()) {
        qDebug() << u"fileIns" << filePath << directoryPath();
        emit fileAdded(filePath);
    }
    return true;
}

void DirectoryManager::removeFileEntry(QString const &filePath)
{
    if (!containsFile(filePath))
        return;
    qsizetype index = indexOfFile(filePath);
    fileEntryVec.erase(fileEntryVec.begin() + index);
    qDebug() << u"fileRem" << filePath;
    emit fileRemoved(filePath, index);
}

void DirectoryManager::updateFileEntry(QString const &filePath)
{
    if (!containsFile(filePath))
        return;
    FSEntry  newEntry(filePath);
    qsizetype index = indexOfFile(filePath);
    if (fileEntryVec[index].modifyTime != newEntry.modifyTime)
        fileEntryVec[index] = newEntry;
    qDebug() << u"fileMod" << filePath;
    emit fileModified(filePath);
}

void DirectoryManager::renameFileEntry(QString const &oldFilePath, QString const &newFileName)
{
    QFileInfo fi(oldFilePath);
    QString   newFilePath = fi.absolutePath() + pathsep + newFileName;
    if (!containsFile(oldFilePath)) {
        if (containsFile(newFilePath))
            updateFileEntry(newFilePath);
        else
            insertFileEntry(newFilePath);
        return;
    }
    if (!isSupportedFile(newFilePath)) {
        removeFileEntry(oldFilePath);
        return;
    }
    if (containsFile(newFilePath)) {
        auto replaceIndex = indexOfFile(newFilePath);
        fileEntryVec.erase(fileEntryVec.begin() + replaceIndex);
        emit fileRemoved(newFilePath, replaceIndex);
    }
    // remove the old one
    auto oldIndex = indexOfFile(oldFilePath);
    fileEntryVec.erase(fileEntryVec.begin() + oldIndex);

    // insert
    auto stdEntry = std::filesystem::directory_entry(util::QStringToStdPath(newFilePath));
    auto fsEntry  = FSEntry(newFilePath, newFileName, stdEntry.file_size(), stdEntry.last_write_time(), stdEntry.is_directory());

    insert_sorted(fileEntryVec, fsEntry, std::bind(compareFunction(), this, std::placeholders::_1, std::placeholders::_2));
    qDebug() << u"fileRen" << oldFilePath << newFilePath;
    emit fileRenamed(oldFilePath, oldIndex, newFilePath, indexOfFile(newFilePath));
}

// ---- dir entries

bool DirectoryManager::insertDirEntry(QString const &dirPath)
{
    if (containsDir(dirPath))
        return false;
    auto stdEntry = std::filesystem::directory_entry(util::QStringToStdPath(dirPath));
    QString dirName = util::StdStringToQString(stdEntry.path().filename().native());

    FSEntry fsEntry;
    fsEntry.name        = dirName;
    fsEntry.path        = dirPath;
    fsEntry.isDirectory = true;
    insert_sorted(dirEntryVec, fsEntry,
                  std::bind(compareFunction(), this, std::placeholders::_1, std::placeholders::_2));
    qDebug() << u"dirIns" << dirPath;
    emit dirAdded(dirPath);
    return true;
}

void DirectoryManager::removeDirEntry(QString const &dirPath)
{
    if (!containsDir(dirPath))
        return;
    qsizetype index = indexOfDir(dirPath);
    dirEntryVec.erase(dirEntryVec.begin() + index);
    qDebug() << u"dirRem" << dirPath;
    emit dirRemoved(dirPath, index);
}

void DirectoryManager::renameDirEntry(QString const &oldDirPath, QString const &newDirName)
{
    if (!containsDir(oldDirPath))
        return;
    QFileInfo fi(oldDirPath);
    QString   newDirPath = fi.absolutePath() + pathsep + newDirName;
    // remove the old one
    auto oldIndex = indexOfDir(oldDirPath);
    dirEntryVec.erase(dirEntryVec.begin() + oldIndex);

    // insert
    FSEntry fsEntry;
    fsEntry.name        = newDirName;
    fsEntry.path        = newDirPath;
    fsEntry.isDirectory = true;
    insert_sorted(dirEntryVec, fsEntry, 
                  std::bind(compareFunction(), this, std::placeholders::_1, std::placeholders::_2));

    qDebug() << u"dirRen" << oldDirPath << newDirPath;
    emit dirRenamed(oldDirPath, oldIndex, newDirPath, indexOfDir(newDirPath));
}


FileListSource DirectoryManager::source() const
{
    return mListSource;
}

QStringList DirectoryManager::fileList() const
{
    QStringList list;
    for (auto const &value : fileEntryVec)
        list << value.path;
    return list;
}

bool DirectoryManager::fileWatcherActive() const
{
    if (!watcher)
        return false;
    return watcher->isObserving();
}

//----------------------------------------------------------------------------
// File system watcher events  ( onFile___External() )
// these take file NAMES, not paths
void DirectoryManager::onFileRemovedExternal(QString const &fileName)
{
    QString fullPath = watcher->watchPath() + pathsep + fileName;
    removeDirEntry(fullPath);
    removeFileEntry(fullPath);
}

void DirectoryManager::onFileAddedExternal(QString const &fileName)
{
    QString fullPath = watcher->watchPath() + pathsep + fileName;
    if (isDir(fullPath))
        insertDirEntry(fullPath);
    else
        insertFileEntry(fullPath);
}

void DirectoryManager::onFileRenamedExternal(QString const &oldFileName, QString const &newFileName)
{
    QString oldPath = watcher->watchPath() + pathsep + oldFileName;
    QString newPath = watcher->watchPath() + pathsep + newFileName;
    if (isDir(newPath))
        renameDirEntry(oldPath, newFileName);
    else
        renameFileEntry(oldPath, newFileName);
}

void DirectoryManager::onFileModifiedExternal(QString const &fileName)
{
    updateFileEntry(watcher->watchPath() + pathsep + fileName);
}
