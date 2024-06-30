#pragma once

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QMimeData>
#include <QImageWriter>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QDesktopWidget>
#endif

#include "Settings.h"
#include "components/actionManager/ActionManager.h"
#include "gui/CentralWidget.h"
#include "gui/customWidgets/FloatingWidgetContainer.h"
#include "gui/dialogs/FileReplaceDialog.h"
#include "gui/dialogs/ResizeDialog.h"
#include "gui/dialogs/SettingsDialog.h"
#include "gui/folderView/FileSystemModelCustom.h"
#include "gui/folderView/FolderViewProxy.h"
#include "gui/overlays/ChangeLogWindow.h"
#include "gui/overlays/ControlsOverlay.h"
#include "gui/overlays/CopyOverlay.h"
#include "gui/overlays/CropOverlay.h"
#include "gui/overlays/FloatingMessageProxy.h"
#include "gui/overlays/FullscreenInfoOverlayProxy.h"
#include "gui/overlays/ImageInfoOverlayProxy.h"
#include "gui/overlays/RenameOverlay.h"
#include "gui/overlays/SaveConfirmOverlay.h"
#include "gui/panels/cropPanel/CropPanel.h"
#include "gui/panels/infoBar/InfoBarProxy.h"
#include "gui/panels/mainPanel/ThumbnailStrip.h"
#include "gui/panels/sidePanel/SidePanel.h"
#include "gui/viewers/DocumentWidget.h"
#include "gui/viewers/ViewerWidget.h"

#ifdef USE_KDE_BLUR
#include <KWindowEffects>
#endif

enum class ActiveSidePanel : uint8_t {
    NONE,
    CROP,
};

class MW final : public FloatingWidgetContainer
{
    Q_OBJECT

    struct CurrentInfo {
        QString   fileName;
        QString   filePath;
        QString   directoryName;
        QString   directoryPath;
        QSize     imageSize = {0, 0};
        qsizetype index     = 0;
        qsizetype fileCount = 0;
        qint64    fileSize  = 0;
        bool      slideshow = false;
        bool      shuffle   = false;
        bool      edited    = false;
    };

  public:
    explicit MW(QWidget *parent);
    ~MW() override;
    DELETE_COPY_MOVE_ROUTINES(MW);

    void onScalingFinished(std::unique_ptr<QPixmap> scaled);
    void showImage(std::unique_ptr<QPixmap> pixmap);
    void showAnimation(QSharedPointer<QMovie> const &movie);
    void showVideo(QString const &file);
    bool showConfirmation(QString const &title, QString const &msg, QMessageBox::StandardButton defaultButton = QMessageBox::Yes);
    void setExifInfo(QMap<QString, QString> const &);

    void setCurrentInfo(qsizetype fileIndex, qsizetype fileCount,
                        QString filePath, QString fileName,
                        QSize imageSize, qint64 fileSize,
                        bool slideshow, bool shuffle, bool edited);

    DialogResult fileReplaceDialog(QString const &source, QString const &dest,
                                   FileReplaceMode mode, bool multiple);

    ND auto currentViewMode() const -> ViewMode;
    ND auto getFolderView() const -> FolderViewProxy *;
    ND auto getThumbnailPanel() const -> ThumbnailStripProxy *;
    ND auto isCropPanelActive() const -> bool;

  private:
    void applyFullscreenBackground();
    void applyWindowedBackground();
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void preShowResize(QSize sz);
    void restoreWindowGeometry();
    void saveCurrentDisplay();
    void saveWindowGeometry();
    void setInteractionEnabled(bool mode);
    void setupCopyOverlay();
    void setupCropPanel();
    void setupRenameOverlay();
    void setupSaveOverlay();
    void setupUi();

  protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    // bool focusNextPrevChild(bool);

  Q_SIGNALS:
    void opened(QString);
    void fullscreenStateChanged(bool);
    void copyRequested(QString);
    void moveRequested(QString);
    void copyUrlsRequested(QList<QString>, QString);
    void moveUrlsRequested(QList<QString>, QString);
    void showFoldersChanged(bool);
    void resizeRequested(QSize);
    void renameRequested(QString);
    void cropRequested(QRect);
    void cropAndSaveRequested(QRect);
    void discardEditsRequested();
    void saveAsClicked();
    void saveRequested();
    void saveAsRequested(QString);
    void sortingSelected(SortingMode);

    // viewerWidget
    void scalingRequested(QSize, ScalingFilter);
    void zoomIn();
    void zoomOut();
    void zoomInCursor();
    void zoomOutCursor();
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void pauseVideo();
    void stopPlayback();
    void seekVideoForward();
    void seekVideoBackward();
    void frameStep();
    void frameStepBack();
    void toggleMute();
    void volumeUp();
    void volumeDown();
    void toggleTransparencyGrid();
    void droppedIn(QMimeData const *, QObject *);
    void draggedOut();
    void setLoopPlayback(bool);
    void playbackFinished();

  public Q_SLOTS:
    void setupFullUi();
    void showDefault();
    void showCropPanel();
    void hideCropPanel();
    void toggleFolderView();
    void enableFolderView();
    void enableDocumentView();
    void showOpenDialog(QString const &path);
    void showSaveDialog(QString const &filePath);
    auto getSaveFileName(QString const &filePath) -> QString;
    void showResizeDialog(QSize initialSize);
    void showSettings();
    void triggerFullScreen();
    void showMessageDirectory(QString const &dirName);
    void showMessageDirectoryEnd();
    void showMessageDirectoryStart();
    void showMessageFitWindow();
    void showMessageFitWidth();
    void showMessageFitOriginal();
    void showFullScreen();
    void showWindowed();
    void triggerCopyOverlay();
    void showMessage(QString const &text);
    void showMessage(QString const &text, int duration);
    void showMessageSuccess(QString const &text);
    void showWarning(QString const &text);
    void showError(QString const &text);
    void triggerMoveOverlay();
    void closeFullScreenOrExit();
    void close();
    void triggerCropPanel();
    void updateCropPanelData();
    void showSaveOverlay();
    void hideSaveOverlay();
    void showChangelogWindow();
    void showChangelogWindow(QString const &text);
    void fitWindow();
    void fitWidth();
    void fitOriginal();
    void switchFitMode();
    void closeImage();
    void showContextMenu();
    void onSortingChanged(SortingMode);
    void toggleImageInfoOverlay();
    void toggleRenameOverlay(QString const &currentName);
    void setFilterNearest();
    void setFilterBilinear();
    void setFilter(ScalingFilter filter);
    void toggleScalingFilter();
    void setDirectoryPath(QString const &path);
    void toggleLockZoom();
    void toggleLockView();
    void toggleFullscreenInfoBar();

  private Q_SLOTS:
    void updateCurrentDisplay();
    void readSettings();
    void adaptToWindowState();
    void onWindowGeometryChanged();
    void onInfoUpdated();
    void showScriptSettings();

  private:
    bool cropPanelActive       : 1 = false;
    bool showInfoBarFullscreen : 1 = false;
    bool showInfoBarWindowed   : 1 = false;
    bool maximized             : 1 = false;

    PanelPosition   panelPosition   = PanelPosition::TOP;
    ActiveSidePanel activeSidePanel = ActiveSidePanel::NONE;
    int             currentDisplay  = 0;

    QHBoxLayout *layout;
    QTimer      *windowGeometryChangeTimer;

    ViewerWidget               *viewerWidget      = nullptr;
    DocumentWidget             *docWidget         = nullptr;
    FolderViewProxy            *folderView        = nullptr;
    CentralWidget              *centralWidget     = nullptr;
    InfoBarProxy               *infoBarWindowed   = nullptr;
    SidePanel                  *sidePanel         = nullptr;
    CropPanel                  *cropPanel         = nullptr;
    CropOverlay                *cropOverlay       = nullptr;
    SaveConfirmOverlay         *saveOverlay       = nullptr;
    ChangelogWindow            *changelogWindow   = nullptr;
    CopyOverlay                *copyOverlay       = nullptr;
    RenameOverlay              *renameOverlay     = nullptr;
    ImageInfoOverlayProxy      *imageInfoOverlay  = nullptr;
    ControlsOverlay            *controlsOverlay   = nullptr;
    FullscreenInfoOverlayProxy *infoBarFullscreen = nullptr;
    FloatingMessageProxy       *floatingMessage   = nullptr;

    CurrentInfo info{};
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QDesktopWidget desktopWidget;
#endif
};
