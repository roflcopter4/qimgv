#pragma once
#ifndef ZRfgG2QRefoVCpmgePTbJ2myqZLSCuRy4bbOv1kitYBTRTrgHx7oh4xfsymuq5S
#define ZRfgG2QRefoVCpmgePTbJ2myqZLSCuRy4bbOv1kitYBTRTrgHx7oh4xfsymuq5S

#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDrag>
#include <QFileSystemModel>
#include <QMutex>
#include <QObject>
#include <QTranslator>

#include "appversion.h"
#include "Settings.h"
#include "components/directorymodel.h"
#include "components/directorypresenter.h"
#include "components/scriptmanager/scriptmanager.h"
#include "gui/mainwindow.h"
#include "utils/randomizer.h"
#include "gui/dialogs/printdialog.h"

#ifdef __GLIBC__
# include <malloc.h>
#endif
#include "Common.h"


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
    enum class MimeDataTarget { TARGET_CLIPBOARD, TARGET_DROP };

    explicit Core(QObject *parent);
    ~Core() override;
    void showGui() const;

    Q_DISABLE_COPY_MOVE(Core)

  public Q_SLOTS:
    void updateInfoString();
    bool loadPath(QString);

  private:
    QElapsedTimer t;

    void initGui();
    void initComponents();
    void connectComponents();
    void initActions();
    void loadTranslation();
    void onUpdate();
    void onFirstRun();

    // ui stuff
    MW   *mw = nullptr;
    State state;
    bool  loopSlideshow;
    bool  slideshow;
    bool  shuffle;

    FolderEndAction folderEndAction;

    // components
    QSharedPointer<DirectoryModel> model;

    DirectoryPresenter *thumbPanelPresenter;
    DirectoryPresenter *folderViewPresenter;

    void rotateByDegrees(int degrees);
    void reset();
    bool setDirectory(QString const &path);

    QDrag       *mDrag;
    QTranslator *translator;
    Randomizer   randomizer;
    QTimer       slideshowTimer;

    void    syncRandomizer();
    void    attachModel(DirectoryModel *newModel);
    QString selectedPath();
    void    guiSetImage(QSharedPointer<Image> const &img);
    void    startSlideshowTimer();
    void    startSlideshow();
    void    stopSlideshow();
    bool    saveFile(QString const &filePath, QString const &newPath);
    bool    saveFile(QString const &filePath);

    ND auto getEditableImage(QString const &filePath) const -> QSharedPointer<ImageStatic>;
    ND auto currentSelection() const -> QList<QString>;

    template <typename... Args>
    void edit_template(bool save, QString const &actionName, std::function<QImage *(QSharedPointer<QImage const>, Args...)> const &editFunc, Args &&...as);

    void doInteractiveCopy(QString const &path, QString const &destDirectory, DialogResult &overwriteFiles);
    void doInteractiveMove(QString const &path, QString const &destDirectory, DialogResult &overwriteFiles);

    static QMimeData *getMimeDataForImage(QSharedPointer<Image> const &img, MimeDataTarget target);

  private Q_SLOTS:
    auto removeFile(QString const &filePath, bool trash) -> FileOpResult;
    void readSettings();
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
    void onFileRemoved(QString const &filePath, int index);
    void onFileRenamed(QString const &fromPath, int indexFrom, const QString &toPath, int indexTo);
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
    bool loadFileIndex(int index, bool async, bool preload);
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


#endif
