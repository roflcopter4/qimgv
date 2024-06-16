#pragma once
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDrag>
#include <QFileSystemModel>
#include <QMutex>
#include <QObject>
#include <QTranslator>

#include "AppVersion.h"
#include "Settings.h"
#include "components/DirectoryModel.h"
#include "components/DirectoryPresenter.h"
#include "components/scriptManager/ScriptManager.h"
#include "gui/MainWindow.h"
#include "gui/dialogs/PrintDialog.h"
#include "utils/Randomizer.h"

#ifdef __GLIBC__
# include <malloc.h>
#endif

class Core final : public QObject
{
    Q_OBJECT

    struct State {
        bool    hasActiveImage = false;
        bool    delayModel     = false;
        QString currentFilePath{};
        QString directoryPath{};
        QSharedPointer<Image> currentImg{};
    };

  public:
    enum class MimeDataTarget {
        TARGET_CLIPBOARD,
        TARGET_DROP,
    };

    explicit Core(QObject *parent);
    ~Core() override;
    DELETE_COPY_MOVE_ROUTINES(Core);

    void showGui() const;

  private:
    void connectComponents();
    void connectActions();
    void loadTranslation();
    void onUpdate();
    void onFirstRun();
    void rotateByDegrees(int degrees);
    void reset();
    bool setDirectory(QString const &path);
    void syncRandomizer();
    void guiSetImage(QSharedPointer<Image> const &img);
    void startSlideshowTimer();
    void startSlideshow();
    void stopSlideshow();
    bool saveFile(QString const &filePath, QString const &newPath);
    bool saveFile(QString const &filePath);

    void doInteractiveCopy(QString const &path, QString const &destDirectory, DialogResult &overwriteFiles);
    void doInteractiveMove(QString const &path, QString const &destDirectory, DialogResult &overwriteFiles);

    ND auto selectedPath() const -> QString;
    ND auto getEditableImage(QString const &filePath) const -> QSharedPointer<ImageStatic>;
    ND auto currentSelection() const -> QList<QString>;

    template <typename... Args>
    void edit_template(bool save,
                       QString const &actionName,
                       std::function<QImage *(QSharedPointer<QImage const>, Args...)> const &editFunc,
                       Args &&...as);

    static QMimeData *getMimeDataForImage(QSharedPointer<Image> const &img, MimeDataTarget target);

    // The entire UI.
    std::unique_ptr<MW> mw;

    // components
    QSharedPointer<DirectoryModel> model;
    DirectoryPresenter *thumbPanelPresenter;
    DirectoryPresenter *folderViewPresenter;

    QTranslator  *translator;
    QDrag        *mDrag;
    Randomizer    randomizer;
    QTimer        slideshowTimer;
    QElapsedTimer t;

    State state;
    FolderEndAction folderEndAction;
    bool  loopSlideshow;
    bool  slideshow;
    bool  shuffle;

  public Q_SLOTS:
    void updateInfoString();
    bool loadPath(QString);

  private Q_SLOTS:
    auto removeFile(QString const &filePath, bool trash) -> FileOpResult;
    void updateSettings();
    void nextImage();
    void prevImage();
    void nextImageSlideshow();
    void jumpToFirst();
    void jumpToLast();
    void onModelItemReady(QSharedPointer<Image> const &, QString const &);
    void onModelItemUpdated(QString const &filePath);
    void onModelSortingChanged(SortingMode mode);
    void onLoadFailed(QString const &path) const;
    void rotateLeft();
    void rotateRight();
    void close() const;
    void scalingRequest(QSize, ScalingFilter) const;
    void onScalingFinished(QPixmap *scaled, ScalerRequest const &req);
    void copyCurrentFile(QString const &destDirectory);
    void moveCurrentFile(QString const &destDirectory);
    void copyPathsTo(QList<QString> const &paths, QString const &destDirectory);
    void interactiveCopy(QList<QString> const &paths, QString const &destDirectory);
    void interactiveMove(QList<QString> const &paths, QString const &destDirectory);
    void movePathsTo(QList<QString> const &paths, QString const &destDirectory);
    void onFileRemoved(QString const &filePath, qsizetype index);
    void onFileRenamed(QString const &fromPath, qsizetype indexFrom, QString const &toPath, qsizetype indexTo);
    void onFileAdded(QString const &filePath);
    void onFileModified(QString const &filePath);
    void showResizeDialog();
    void resize(QSize size);
    void flipH();
    void flipV();
    void crop(QRect rect);
    void cropAndSave(QRect rect);
    void discardEdits();
    void toggleCropPanel();
    void toggleFullscreenInfoBar();
    void requestSavePath();
    void saveCurrentFile();
    void saveCurrentFileAs(QString const &);
    void runScript(QString const &);
    void setWallpaper();
    void removePermanent();
    void moveToTrash();
    void reloadImage();
    void reloadImage(QString const &filePath);
    void copyFileClipboard();
    void copyPathClipboard();
    void openFromClipboard();
    void renameCurrentSelection(QString const &newName);
    void sortBy(SortingMode mode) const;
    void sortByName();
    void sortByTime();
    void sortBySize();
    void showRenameDialog();
    void onDraggedOut();
    void onDraggedOut(QList<QString> const &paths);
    void onDropIn(QMimeData const *mimeData, QObject const *source);
    void toggleShuffle();
    void onModelLoaded();
    void outputError(FileOpResult const &error) const;
    void showOpenDialog() const;
    void showInDirectory();
    void onDirectoryViewFileActivated(QString const &filePath);
    bool loadFileIndex(qsizetype index, bool async, bool preload);
    void enableDocumentView();
    void enableFolderView();
    void toggleFolderView();
    void toggleSlideshow();
    void onPlaybackFinished();
    void setFoldersDisplay(bool mode);
    void loadParentDir();
    void nextDirectory();
    void prevDirectory(bool selectLast);
    void prevDirectory();
    void print();
    void modelDelayLoad();
};
