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

#include "gui/customwidgets/floatingwidgetcontainer.h"
#include "gui/viewers/viewerwidget.h"
#include "gui/overlays/controlsoverlay.h"
#include "gui/overlays/fullscreeninfooverlayproxy.h"
#include "gui/overlays/floatingmessageproxy.h"
#include "gui/overlays/saveconfirmoverlay.h"
#include "gui/panels/mainpanel/thumbnailstrip.h"
#include "gui/panels/sidepanel/sidepanel.h"
#include "gui/panels/croppanel/croppanel.h"
#include "gui/overlays/cropoverlay.h"
#include "gui/overlays/copyoverlay.h"
#include "gui/overlays/changelogwindow.h"
#include "gui/overlays/imageinfooverlayproxy.h"
#include "gui/overlays/renameoverlay.h"
#include "gui/dialogs/resizedialog.h"
#include "gui/centralwidget.h"
#include "gui/dialogs/filereplacedialog.h"
#include "components/actionmanager/actionmanager.h"
#include "settings.h"
#include "gui/dialogs/settingsdialog.h"
#include "gui/viewers/documentwidget.h"
#include "gui/folderview/folderviewproxy.h"
#include "gui/panels/infobar/infobarproxy.h"

#ifdef USE_KDE_BLUR
#include <KWindowEffects>
#endif

#include "Common.h"

struct CurrentInfo {
    qsizetype index;
    qsizetype fileCount;
    QString   fileName;
    QString   filePath;
    QString   directoryName;
    QString   directoryPath;
    QSize     imageSize;
    qint64    fileSize;
    bool      slideshow;
    bool      shuffle;
    bool      edited;
};

enum class ActiveSidePanel : uint8_t {
    NONE,
    CROP,
};

class MW Q_DECL_FINAL : public FloatingWidgetContainer
{
    Q_OBJECT

  public:
    explicit MW(QWidget *parent = nullptr);
    ~MW() override;

    bool isCropPanelActive() const;
    void onScalingFinished(std::unique_ptr<QPixmap> scaled);
    void showImage(std::unique_ptr<QPixmap> pixmap);
    void showAnimation(QSharedPointer<QMovie> const &movie);
    void showVideo(QString const &file);

    void setCurrentInfo(qsizetype fileIndex, qsizetype fileCount, QString const &filePath, QString const &fileName, QSize imageSize, qint64 fileSize, bool slideshow, bool shuffle, bool edited);
    void setExifInfo(QMap<QString, QString> const &);

    QSharedPointer<FolderViewProxy>     getFolderView();
    QSharedPointer<ThumbnailStripProxy> getThumbnailPanel();

    ND ViewMode currentViewMode() const;

    bool showConfirmation(QString const &title, QString const &msg);

    DialogResult fileReplaceDialog(QString const &source, QString const &dest, FileReplaceMode mode, bool multiple);

  private:
    int  currentDisplay;
    bool cropPanelActive;
    bool showInfoBarFullscreen;
    bool showInfoBarWindowed;
    bool maximized;

    PanelPosition   panelPosition;
    ActiveSidePanel activeSidePanel;

    QTimer          *windowGeometryChangeTimer;
    QHBoxLayout     *layout;

    QSharedPointer<ViewerWidget>    viewerWidget;
    QSharedPointer<DocumentWidget>  docWidget;
    QSharedPointer<FolderViewProxy> folderView;
    QSharedPointer<CentralWidget>   centralWidget;
    QSharedPointer<InfoBarProxy>    infoBarWindowed;

    SidePanel                  *sidePanel;
    CropPanel                  *cropPanel;
    CropOverlay                *cropOverlay;
    SaveConfirmOverlay         *saveOverlay;
    ChangelogWindow            *changelogWindow;
    CopyOverlay                *copyOverlay;
    RenameOverlay              *renameOverlay;
    ImageInfoOverlayProxy      *imageInfoOverlay;
    ControlsOverlay            *controlsOverlay;
    FullscreenInfoOverlayProxy *infoBarFullscreen;
    FloatingMessageProxy       *floatingMessage;

    CurrentInfo info;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QDesktopWidget desktopWidget;
#endif

    void saveWindowGeometry();
    void restoreWindowGeometry();
    void saveCurrentDisplay();
    void setupUi();

    void applyWindowedBackground();
    void applyFullscreenBackground();
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void setupCropPanel();
    void setupCopyOverlay();
    void setupSaveOverlay();
    void setupRenameOverlay();
    void preShowResize(QSize sz);
    void setInteractionEnabled(bool mode);

  private slots:
    void updateCurrentDisplay();
    void readSettings();
    void adaptToWindowState();
    void onWindowGeometryChanged();
    void onInfoUpdated();
    void showScriptSettings();

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
  signals:
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
    void droppedIn(const QMimeData *, QObject *);
    void draggedOut();
    void setLoopPlayback(bool);
    void playbackFinished();

  public slots:
    void setupFullUi();
    void showDefault();
    void showCropPanel();
    void hideCropPanel();
    void toggleFolderView();
    void enableFolderView();
    void enableDocumentView();
    void showOpenDialog(QString const &path);
    void showSaveDialog(QString const &filePath);
    auto getSaveFileName(QString const &fileName) -> QString;
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
};
