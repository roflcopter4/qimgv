#pragma once

#include <QObject>
#include "cache/Cache.h"
#include "directoryManager/DirectoryManager.h"
#include "scaler/Scaler.h"
#include "loader/Loader.h"
#include "utils/FileOperations.h"

class DirectoryModel final : public QObject
{
    Q_OBJECT

  public:
    explicit DirectoryModel(QObject *parent);
    ~DirectoryModel() override;

    void load(QString const &filePath, bool asyncHint);
    void preload(QString const &filePath);
    void unload(qsizetype index);
    void unload(QString const &filePath);
    void reload(QString const &filePath);
    void unloadExcept(QString const &filePath, bool keepNearby);

    bool forceInsert(QString const &filePath);
    void moveFileTo(QString const &srcFile, QString const &destDirPath, bool force, FileOpResult &result);
    void renameEntry(QString const &oldPath, QString const &newName, bool force, FileOpResult &result);
    void removeFile(QString const &filePath, bool trash, FileOpResult &result);
    void removeDir(QString const &dirPath, bool trash, bool recursive, FileOpResult &result);
    bool setDirectory(QString const &);
    bool saveFile(QString const &filePath);
    bool saveFile(QString const &filePath, QString const &destPath);
    void updateImage(QString const &filePath, QSharedPointer<Image> const &img);

    ND qsizetype fileCount() const;
    ND qsizetype dirCount() const;
    ND qsizetype totalCount() const;
    ND qsizetype indexOfFile(QString const &filePath) const;
    ND qsizetype indexOfDir(QString const &filePath) const;

    ND QString dirNameAt(qsizetype index) const;
    ND QString dirPathAt(qsizetype index) const;
    ND QString fileNameAt(qsizetype index) const;
    ND QString filePathAt(qsizetype index) const;
    ND QString directoryPath() const;
    ND QString firstFile() const;
    ND QString lastFile() const;
    ND QString nextOf(QString const &filePath) const;
    ND QString prevOf(QString const &filePath) const;

    ND auto fileEntryAt(qsizetype index) const -> FSEntry const &;
    ND auto getImage(QString const &filePath) const -> QSharedPointer<Image>;
    ND auto getImageAt(qsizetype index) const -> QSharedPointer<Image>;
    ND auto lastModified(QString const &filePath) const -> QDateTime;
    ND auto source() const -> FileListSource;
    ND bool autoRefresh() const;
    ND bool containsDir(QString const &dirPath) const;
    ND bool containsFile(QString const &filePath) const;
    ND bool isEmpty() const;
    ND bool isLoaded(QString const &filePath) const;
    ND bool isLoaded(qsizetype index) const;
    ND bool loaderBusy() const;

    ND auto sortingMode() const -> SortingMode;
       void setSortingMode(SortingMode mode);

    static void copyFileTo(QString const &srcFile, QString const &destDirPath, bool force, FileOpResult &result);

    Q_DISABLE_COPY_MOVE(DirectoryModel)

    Scaler *scaler;

  Q_SIGNALS:
    void fileRemoved(QString filePath, qsizetype index);
    void fileRenamed(QString fromPath, qsizetype indexFrom, QString toPath, qsizetype indexTo);
    void fileAdded(QString filePath);
    void fileModified(QString filePath);
    void dirRemoved(QString dirPath, qsizetype index);
    void dirRenamed(QString dirPath, qsizetype indexFrom, QString toPath, qsizetype indexTo);
    void dirAdded(QString dirPath);
    void loaded(QString filePath);
    void loadFailed(QString const &path);
    void sortingChanged(SortingMode);
    void indexChanged(qsizetype oldIndex, qsizetype index);
    void imageReady(QSharedPointer<Image> img, QString const &);
    void imageUpdated(QString filePath);

  private:
    DirectoryManager *dirManager;

    Loader         loader;
    Cache          cache;
    FileListSource fileListSource;

  private Q_SLOTS:
    void onImageReady(QSharedPointer<Image> const &img, QString const &path);
    void onSortingChanged();
    void onFileAdded(QString const &filePath);
    void onFileRemoved(QString const &filePath, qsizetype index);
    void onFileRenamed(QString const &fromPath, qsizetype indexFrom, QString const &toPath, qsizetype indexTo);
    void onFileModified(QString const &filePath);
};
