#pragma once

#include "ThemeStore.h"
#include "utils/Script.h"
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFont>
#include <QFontMetrics>
#include <QImageReader>
#include <QKeySequence>
#include <QMap>
#include <QObject>
#include <QPalette>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QThread>
#include <QVersionNumber>

enum class SortingMode : uint8_t {
    NAME,
    NAME_DESC,
    SIZE,
    SIZE_DESC,
    TIME,
    TIME_DESC,
};

enum class ImageFitMode : uint8_t {
    WINDOW,
    WIDTH,
    ORIGINAL,
    FREE,
};

enum class PanelPosition : uint8_t {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
};

enum class ScalingFilter : uint8_t {
    NEAREST,
    BILINEAR,
    CV_BILINEAR_SHARPEN,
    CV_CUBIC,
    CV_CUBIC_SHARPEN,
};

enum class ZoomIndicatorMode : uint8_t {
    DISABLED,
    ENABLED,
    AUTO,
};

enum class DefaultCropAction : uint8_t {
    CROP,
    CROP_SAVE,
};

enum class ImageFocusPoint : uint8_t {
    TOP,
    CENTER,
    CURSOR,
};

enum class ImageScrolling : uint8_t {
    NONE,
    BY_TRACKPAD,
    BY_TRACKPAD_AND_WHEEL,
};

enum class ViewMode : uint8_t {
    INVALID,
    DOCUMENT,
    FOLDERVIEW,
};

enum class FolderEndAction : uint8_t {
    NOTHING,
    LOOP,
    GOTO_ADJACENT,
};

enum class FolderViewMode : uint8_t {
    SIMPLE,
    EXTENDED,
    EXT_FOLDERS,
};

enum class ThumbPanelStyle : uint8_t {
    SIMPLE,
    EXTENDED,
};


class Settings final : public QObject
{
    Q_OBJECT

  public:
    static Settings *getInstance();
    ~Settings() override;
    DELETE_COPY_MOVE_ROUTINES(Settings);

    void loadStylesheet();
    void readShortcuts(QMap<QString, QString> &shortcuts);
    void saveShortcuts(QMap<QString, QString> const &shortcuts);
    void readScripts(QMap<QString, Script> &scripts);
    void saveScripts(QMap<QString, Script> const &scripts);
    void sync();

    ND auto supportedMimeTypes() const -> QStringList;
    ND auto supportedFormats() const -> QList<QByteArray>;
    ND auto supportedFormatsFilter() const -> QString;
    ND auto supportedFormatsRegex() const -> QString;
    ND auto tmpDir() const -> QString;
    ND auto thumbnailCacheDir() const -> QString;
    ND auto videoFormats() const -> QMultiMap<QByteArray, QByteArray> const &;
    ND bool unlockMinZoom() const;

    ND bool printLandscape() const;
       void setPrintLandscape(bool mode);
    ND bool printPdfDefault() const;
       void setPrintPdfDefault(bool mode);
    ND bool printColor() const;
       void setPrintColor(bool mode);
    ND bool printFitToPage() const;
       void setPrintFitToPage(bool mode);
    ND auto lastPrinter() const -> QString;
       void setLastPrinter(QString const &name);
    ND auto language() const -> QString;
       void setLanguage(QString const &lang);
    ND auto zoomLevels() const -> QString;
       void setZoomLevels(QString const &levels);
    ND auto colorScheme() const -> ColorScheme const &;
       void setColorScheme(ColorScheme const &scheme);
       void setColorTid(int tid);
       void setColorTid(ColorSchemes tid);

    ND auto mpvBinary() const -> QString;
       void setMpvBinary(QString const &path);
    ND int  panelPreviewsSize() const;
       void setPanelPreviewsSize(int size);
    ND bool usePreloader() const;
       void setUsePreloader(bool mode);
    ND bool fullscreenMode() const;
       void setFullscreenMode(bool mode);
    ND auto imageFitMode() const -> ImageFitMode;
       void setImageFitMode(ImageFitMode mode);
    ND auto windowGeometry() const -> QRect;
       void setWindowGeometry(QRect geometry);
    ND bool playVideoSounds() const;
       void setPlayVideoSounds(bool mode);
    ND int  volume() const;
       void setVolume(int vol);
    ND auto panelPosition() const -> PanelPosition;
       void setPanelPosition(PanelPosition);
    ND bool loopSlideshow() const;
       void setLoopSlideshow(bool mode);
    ND bool panelEnabled() const;
       void setPanelEnabled(bool mode);
    ND int  lastDisplay() const;
       void setLastDisplay(int display);
    ND bool squareThumbnails() const;
       void setSquareThumbnails(bool mode);
    ND bool transparencyGrid() const;
       void setTransparencyGrid(bool mode);
    ND bool enableSmoothScroll() const;
       void setEnableSmoothScroll(bool mode);
    ND bool useThumbnailCache() const;
       void setUseThumbnailCache(bool mode);
    ND auto savedPaths() const -> QStringList;
       void setSavedPaths(QStringList const &paths);
    ND int  thumbnailerThreadCount() const;
       void setThumbnailerThreadCount(int count);
    ND bool smoothUpscaling() const;
       void setSmoothUpscaling(bool mode);
    ND bool expandImage() const;
       void setExpandImage(bool mode);
    ND auto scalingFilter() const -> ScalingFilter;
       void setScalingFilter(ScalingFilter mode);
    ND bool smoothAnimatedImages() const;
       void setSmoothAnimatedImages(bool mode);
    ND bool panelFullscreenOnly() const;
       void setPanelFullscreenOnly(bool mode);
    ND auto lastVersion() const -> QVersionNumber;
       void setLastVersion(QVersionNumber const &ver);
    ND bool showChangelogs() const;
       void setShowChangelogs(bool mode);
    ND auto backgroundOpacity() const -> qreal;
       void setBackgroundOpacity(qreal value);
    ND bool blurBackground() const;
       void setBlurBackground(bool mode);
    ND auto sortingMode() const -> SortingMode;
       void setSortingMode(SortingMode mode);
    ND int  folderViewIconSize() const;
       void setFolderViewIconSize(int value);
    ND bool firstRun() const;
       void setFirstRun(bool mode);
    ND bool cursorAutohide() const;
       void setCursorAutohide(bool mode);
    ND bool infoBarFullscreen() const;
       void setInfoBarFullscreen(bool mode);
    ND bool infoBarWindowed() const;
       void setInfoBarWindowed(bool mode);
    ND bool windowTitleExtendedInfo() const;
       void setWindowTitleExtendedInfo(bool mode);
    ND bool maximizedWindow() const;
       void setMaximizedWindow(bool mode);
    ND bool keepFitMode() const;
       void setKeepFitMode(bool mode);
    ND int  expandLimit() const;
       void setExpandLimit(int value);
    ND auto zoomStep() const -> float;
       void setZoomStep(float value);
    ND int  JPEGSaveQuality() const;
       void setJPEGSaveQuality(int value);
    ND bool placesPanel() const;
       void setPlacesPanel(bool mode);
    ND bool placesPanelBookmarksExpanded() const;
       void setPlacesPanelBookmarksExpanded(bool mode);
    ND bool placesPanelTreeExpanded() const;
       void setPlacesPanelTreeExpanded(bool mode);
    ND int  slideshowInterval() const;
       void setSlideshowInterval(int ms);
    ND int  placesPanelWidth() const;
       void setPlacesPanelWidth(int width);
    ND auto bookmarks() const -> QStringList;
       void setBookmarks(QStringList const &paths);
    ND auto zoomIndicatorMode() const -> ZoomIndicatorMode;
       void setZoomIndicatorMode(ZoomIndicatorMode mode);
    ND auto focusPointIn1to1Mode() const -> ImageFocusPoint;
       void setFocusPointIn1to1Mode(ImageFocusPoint mode);
    ND auto defaultCropAction() const -> DefaultCropAction;
       void setDefaultCropAction(DefaultCropAction mode);
    ND auto imageScrolling() const -> ImageScrolling;
       void setImageScrolling(ImageScrolling mode);
    ND auto defaultViewMode() const -> ViewMode;
       void setDefaultViewMode(ViewMode mode);
    ND auto folderEndAction() const -> FolderEndAction;
       void setFolderEndAction(FolderEndAction mode);
    ND bool videoPlayback() const;
       void setVideoPlayback(bool mode);
    ND bool showSaveOverlay() const;
       void setShowSaveOverlay(bool mode);
    ND bool confirmDelete() const;
       void setConfirmDelete(bool mode);
    ND bool confirmTrash() const;
       void setConfirmTrash(bool mode);
    ND auto folderViewMode() const -> FolderViewMode;
       void setFolderViewMode(FolderViewMode mode);
    ND bool unloadThumbs() const;
       void setUnloadThumbs(bool mode);
    ND auto thumbPanelStyle() const -> ThumbPanelStyle;
       void setThumbPanelStyle(ThumbPanelStyle mode);
    ND bool jxlAnimation() const;
       void setJxlAnimation(bool mode);
    ND bool absoluteZoomStep();
       void setAbsoluteZoomStep(bool mode);
    ND bool autoResizeWindow() const;
       void setAutoResizeWindow(bool mode);
    ND int  autoResizeLimit() const;
       void setAutoResizeLimit(int percent);
    ND bool panelPinned() const;
       void setPanelPinned(bool mode);
    ND int  memoryAllocationLimit() const;
       void setMemoryAllocationLimit(int limitMB);
    ND bool panelCenterSelection() const;
       void setPanelCenterSelection(bool mode);
       void setUseFixedZoomLevels(bool mode);
    ND bool useFixedZoomLevels() const;
       void setUnlockMinZoom(bool mode);
    ND bool sortFolders() const;
       void setSortFolders(bool mode);
    ND bool trackpadDetection() const;
       void setTrackpadDetection(bool mode);
    ND bool useSystemColorScheme() const;
       void setUseSystemColorScheme(bool mode);
    ND bool clickableEdges();
       void setClickableEdges(bool mode);
    ND bool clickableEdgesVisible();
       void setClickableEdgesVisible(bool mode);

    ND static QString defaultZoomLevels();

  private:
    explicit Settings(QObject *parent = nullptr);

    void loadTheme();
    void saveTheme();
    void createColorVariants();
    void setupCache();
    void fillVideoFormats();

  Q_SIGNALS:
    void settingsChanged();

  public Q_SLOTS:
    void sendChangeNotification();

  private:
#ifndef Q_OS_LINUX
    QDir mConfDir;
#endif
    QDir mTmpDir;
    QDir mThumbCacheDir;

    ColorScheme mColorScheme;
    QSettings   settingsConf;
    QSettings   stateConf;
    QSettings   themeConf;

    QMultiMap<QByteArray, QByteArray> mVideoFormatsMap; // [mimetype, format]
};

extern Settings *settings;
