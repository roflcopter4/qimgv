#include "directorymodel.h"

DirectoryModel::DirectoryModel(QObject *parent)
    : QObject(parent),
      scaler(new Scaler(&cache)),
      fileListSource(FileListSource::DIRECTORY)
{
    connect(&dirManager, &DirectoryManager::fileRemoved, this, &DirectoryModel::onFileRemoved);
    connect(&dirManager, &DirectoryManager::fileAdded, this, &DirectoryModel::onFileAdded);
    connect(&dirManager, &DirectoryManager::fileRenamed, this, &DirectoryModel::onFileRenamed);
    connect(&dirManager, &DirectoryManager::fileModified, this, &DirectoryModel::onFileModified);
    connect(&dirManager, &DirectoryManager::dirRemoved, this, &DirectoryModel::dirRemoved);
    connect(&dirManager, &DirectoryManager::dirAdded, this, &DirectoryModel::dirAdded);
    connect(&dirManager, &DirectoryManager::dirRenamed, this, &DirectoryModel::dirRenamed);

    connect(&dirManager, &DirectoryManager::loaded, this, &DirectoryModel::loaded);
    connect(&dirManager, &DirectoryManager::sortingChanged, this, &DirectoryModel::onSortingChanged);
    connect(&loader, &Loader::loadFinished, this, &DirectoryModel::onImageReady);
    connect(&loader, &Loader::loadFailed, this, &DirectoryModel::loadFailed);
}

DirectoryModel::~DirectoryModel()
{
    loader.clearTasks();
    delete scaler;
}

qsizetype DirectoryModel::totalCount() const
{
    return dirManager.totalCount();
}

qsizetype DirectoryModel::fileCount() const
{
    return dirManager.fileCount();
}

qsizetype DirectoryModel::dirCount() const
{
    return dirManager.dirCount();
}

qsizetype DirectoryModel::indexOfFile(QString const &filePath) const
{
    return dirManager.indexOfFile(filePath);
}

qsizetype DirectoryModel::indexOfDir(QString const &filePath) const
{
    return dirManager.indexOfDir(filePath);
}

SortingMode DirectoryModel::sortingMode() const
{
    return dirManager.sortingMode();
}

FSEntry const &DirectoryModel::fileEntryAt(qsizetype index) const
{
    return dirManager.fileEntryAt(index);
}

QString DirectoryModel::fileNameAt(qsizetype index) const
{
    return dirManager.fileNameAt(index);
}

QString DirectoryModel::filePathAt(qsizetype index) const
{
    return dirManager.filePathAt(index);
}

QString DirectoryModel::dirNameAt(qsizetype index) const
{
    return dirManager.dirNameAt(index);
}

QString DirectoryModel::dirPathAt(qsizetype index) const
{
    return dirManager.dirPathAt(index);
}

bool DirectoryModel::autoRefresh() const
{
    return dirManager.fileWatcherActive();
}

FileListSource DirectoryModel::source() const
{
    return dirManager.source();
}

QString DirectoryModel::directoryPath() const
{
    return dirManager.directoryPath();
}

bool DirectoryModel::containsFile(QString const &filePath) const
{
    return dirManager.containsFile(filePath);
}

bool DirectoryModel::containsDir(QString const &dirPath) const
{
    return dirManager.containsDir(dirPath);
}

bool DirectoryModel::isEmpty() const
{
    return dirManager.isEmpty();
}

QString DirectoryModel::firstFile() const
{
    return dirManager.firstFile();
}

QString DirectoryModel::lastFile() const
{
    return dirManager.lastFile();
}

QString DirectoryModel::nextOf(QString const &filePath) const
{
    return dirManager.nextOfFile(filePath);
}

QString DirectoryModel::prevOf(QString const &filePath) const
{
    return dirManager.prevOfFile(filePath);
}

QDateTime DirectoryModel::lastModified(QString const &filePath) const
{
    return dirManager.lastModified(filePath);
}

// -----------------------------------------------------------------------------

bool DirectoryModel::forceInsert(QString const &filePath)
{
    return dirManager.forceInsertFileEntry(filePath);
}

void DirectoryModel::setSortingMode(SortingMode mode)
{
    dirManager.setSortingMode(mode);
}

void DirectoryModel::removeFile(QString const &filePath, bool trash, FileOpResult &result)
{
    if (trash)
        FileOperations::moveToTrash(filePath, result);
    else
        FileOperations::removeFile(filePath, result);
    if (result != FileOpResult::SUCCESS)
        return;
    dirManager.removeFileEntry(filePath);
}

void DirectoryModel::renameEntry(QString const &oldPath, QString const &newName, bool force, FileOpResult &result)
{
    bool isDir = DirectoryManager::isDir(oldPath);
    FileOperations::rename(oldPath, newName, force, result);
    // Chew through watcher events so they won't be processed out of order.
    qApp->processEvents();
    if (result != FileOpResult::SUCCESS)
        return;
    if (isDir)
        dirManager.renameDirEntry(oldPath, newName);
    else
        dirManager.renameFileEntry(oldPath, newName);
}

void DirectoryModel::removeDir(QString const &dirPath, bool trash, bool recursive, FileOpResult &result)
{
    if (trash)
        FileOperations::moveToTrash(dirPath, result);
    else
        FileOperations::removeDir(dirPath, recursive, result);
    if (result != FileOpResult::SUCCESS)
        return;
    dirManager.removeDirEntry(dirPath);
}

void DirectoryModel::copyFileTo(QString const &srcFile, QString const &destDirPath, bool force, FileOpResult &result)
{
    FileOperations::copyFileTo(srcFile, destDirPath, force, result);
}

void DirectoryModel::moveFileTo(QString const &srcFile, QString const &destDirPath, bool force, FileOpResult &result)
{
    FileOperations::moveFileTo(srcFile, destDirPath, force, result);
    // Chew through watcher events so they won't be processed out of order.
    qApp->processEvents();
    if (result == FileOpResult::SUCCESS) {
        if (destDirPath != this->directoryPath())
            dirManager.removeFileEntry(srcFile);
    }
}
// -----------------------------------------------------------------------------
bool DirectoryModel::setDirectory(QString const &path)
{
    cache.clear();
    return dirManager.setDirectory(path);
}

void DirectoryModel::unload(qsizetype index)
{
    QString filePath = this->filePathAt(index);
    cache.remove(filePath);
}

void DirectoryModel::unload(QString const &filePath)
{
    cache.remove(filePath);
}

void DirectoryModel::unloadExcept(QString const &filePath, bool keepNearby)
{
    QList<QString> list;
    list << filePath;
    if (keepNearby) {
        list << prevOf(filePath);
        list << nextOf(filePath);
    }
    cache.trimTo(list);
}

bool DirectoryModel::loaderBusy() const
{
    return loader.isBusy();
}

void DirectoryModel::onImageReady(std::shared_ptr<Image> const &img, QString const &path)
{
    if (!img) {
        emit loadFailed(path);
        return;
    }
    cache.remove(path);
    cache.insert(img);
    emit imageReady(img, path);
}

bool DirectoryModel::saveFile(QString const &filePath)
{
    return saveFile(filePath, filePath);
}

bool DirectoryModel::saveFile(QString const &filePath, QString const &destPath)
{
    if (!containsFile(filePath) || !cache.contains(filePath))
        return false;
    auto img = cache.get(filePath);
    if (img->save(destPath)) {
        if (filePath == destPath) { // replace
            dirManager.updateFileEntry(destPath);
            emit fileModified(destPath);
        } else { // manually add if we are saving to the same dir
            QFileInfo fiSrc(filePath);
            QFileInfo fiDest(destPath);
            // handle same dir
            if (fiSrc.absolutePath() == fiDest.absolutePath()) {
                // overwrite
                if (!dirManager.containsFile(destPath) && dirManager.insertFileEntry(destPath))
                    emit fileModified(destPath);
            }
        }
        return true;
    }
    return false;
}

// dirManager events

void DirectoryModel::onSortingChanged()
{
    emit sortingChanged(sortingMode());
}

void DirectoryModel::onFileAdded(QString const &filePath)
{
    emit fileAdded(filePath);
}

void DirectoryModel::onFileModified(QString const &filePath)
{
    QDateTime modTime = lastModified(filePath);
    if (modTime.isValid()) {
        if (auto const &img = cache.get(filePath)) {
            // check if file on disk is different
            if (modTime != img->lastModified())
                reload(filePath);
        }
        emit fileModified(filePath);
    }
}

void DirectoryModel::onFileRemoved(QString const &filePath, qsizetype index)
{
    unload(filePath);
    emit fileRemoved(filePath, index);
}

void DirectoryModel::onFileRenamed(QString const &fromPath, qsizetype indexFrom, QString const &toPath, qsizetype indexTo)
{
    unload(fromPath);
    emit fileRenamed(fromPath, indexFrom, toPath, indexTo);
}

bool DirectoryModel::isLoaded(qsizetype index) const
{
    return cache.contains(filePathAt(index));
}

bool DirectoryModel::isLoaded(QString const &filePath) const
{
    return cache.contains(filePath);
}

std::shared_ptr<Image> DirectoryModel::getImageAt(qsizetype index) const
{
    return getImage(filePathAt(index));
}

// returns cached image
// if image is not cached, loads it in the main thread
// for async access use loadAsync(), then catch onImageReady()
std::shared_ptr<Image> DirectoryModel::getImage(QString const &filePath) const
{
    std::shared_ptr<Image> img = cache.get(filePath);
    if (!img)
        img = Loader::load(filePath);
    return img;
}

void DirectoryModel::updateImage(QString const &filePath, std::shared_ptr<Image> const &img)
{
    if (containsFile(filePath) /*& cache.contains(filePath)*/) {
        if (!cache.contains(filePath)) {
            cache.insert(img);
        } else {
            cache.insert(img);
            emit imageUpdated(filePath);
        }
    }
}

void DirectoryModel::load(QString const &filePath, bool asyncHint)
{
    if (!containsFile(filePath) || loader.isLoading(filePath))
        return;
    if (!cache.contains(filePath)) {
        if (asyncHint) {
            loader.loadAsyncPriority(filePath);
        } else if (auto img = loader.load(filePath)) {
            cache.insert(img);
            emit imageReady(img, filePath);
        } else {
            emit loadFailed(filePath);
        }
    } else {
        emit imageReady(cache.get(filePath), filePath);
    }
}

void DirectoryModel::reload(QString const &filePath)
{
    if (cache.contains(filePath)) {
        cache.remove(filePath);
        dirManager.updateFileEntry(filePath);
        load(filePath, false);
    }
}

void DirectoryModel::preload(QString const &filePath)
{
    if (containsFile(filePath) && !cache.contains(filePath))
        loader.loadAsync(filePath);
}
