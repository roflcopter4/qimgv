#pragma once

#include <QObject>
#include "cache/cache.h"
#include "directorymanager/directorymanager.h"
#include "scaler/scaler.h"
#include "loader/loader.h"
#include "utils/fileoperations.h"

#include "Common.h"

class DirectoryModel : public QObject
{
    Q_OBJECT

public:
    explicit DirectoryModel(QObject *parent = nullptr);
    ~DirectoryModel() override;

    Scaler *scaler;

    void load(QString const &filePath, bool asyncHint);
    void preload(QString const &filePath);

    ND int fileCount() const;
    ND int dirCount() const;
    ND int totalCount() const;
    ND int indexOfFile(QString const &filePath) const;
    ND int indexOfDir(QString const &filePath) const;

    ND QString   fileNameAt(int index) const;
    ND bool      containsFile(QString const &filePath) const;
    ND bool      isEmpty() const;
    ND QString   nextOf(QString const &filePath) const;
    ND QString   prevOf(QString const &filePath) const;
    ND QString   firstFile() const;
    ND QString   lastFile() const;
    ND QDateTime lastModified(QString const &filePath) const;

    static void copyFileTo(QString const &srcFile, QString const &destDirPath, bool force, FileOpResult &result);
    bool forceInsert(QString const &filePath);
    void moveFileTo(QString const &srcFile, QString const &destDirPath, bool force, FileOpResult &result);
    void renameEntry(QString const &oldPath, QString const &newName, bool force, FileOpResult &result);
    void removeFile(QString const &filePath, bool trash, FileOpResult &result);
    void removeDir(QString const &dirPath, bool trash, bool recursive, FileOpResult &result);

    bool    setDirectory(QString const &);
    void    unload(int index);
    ND bool loaderBusy() const;

    std::shared_ptr<Image> getImageAt(int index);
    std::shared_ptr<Image> getImage(QString const &filePath);

    void updateImage(QString const &filePath, std::shared_ptr<Image> const &img);

    ND SortingMode sortingMode() const;
    void setSortingMode(SortingMode mode);
    void unload(QString const &filePath);
    void reload(QString const &filePath);
    void unloadExcept(QString const &filePath, bool keepNearby);

    ND bool isLoaded(int index) const;
    ND bool isLoaded(QString const &filePath) const;

    ND QString directoryPath() const;
    ND QString filePathAt(int index) const;
    ND FSEntry const &fileEntryAt(int index) const;
    ND QString dirNameAt(int index) const;
    ND QString dirPathAt(int index) const;

    bool saveFile(QString const &filePath);
    bool saveFile(QString const &filePath, QString const &destPath);

    ND bool autoRefresh() const;
    ND bool containsDir(QString const &dirPath) const;
    ND FileListSource source() const;

signals:
    void fileRemoved(QString filePath, int index);
    void fileRenamed(QString fromPath, int indexFrom, QString toPath, int indexTo);
    void fileAdded(QString filePath);
    void fileModified(QString filePath);
    void dirRemoved(QString dirPath, int index);
    void dirRenamed(QString dirPath, int indexFrom, QString toPath, int indexTo);
    void dirAdded(QString dirPath);
    void loaded(QString filePath);
    void loadFailed(QString const &path);
    void sortingChanged(SortingMode);
    void indexChanged(int oldIndex, int index);
    void imageReady(std::shared_ptr<Image> img, QString const &);
    void imageUpdated(QString filePath);

private:
    DirectoryManager dirManager;
    Loader loader;
    Cache cache;
    FileListSource fileListSource;

private slots:
    void onImageReady(std::shared_ptr<Image> const &img, QString const &path);
    void onSortingChanged();
    void onFileAdded(QString const &filePath);
    void onFileRemoved(QString const &filePath, int index);
    void onFileRenamed(QString const &fromPath, int indexFrom, QString const &toPath, int indexTo);
    void onFileModified(QString const &filePath);
};
