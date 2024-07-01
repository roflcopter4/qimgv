#pragma once

#include "gui/ContextMenu.h"
#include "gui/customWidgets/FloatingWidgetContainer.h"
#include "gui/overlays/VideoControlsProxy.h"
#include "gui/overlays/ZoomIndicatorOverlayProxy.h"
#include "gui/viewers/ImageViewerV2.h"
#include "gui/viewers/VideoPlayerInitProxy.h"
#include <QVBoxLayout>

class ViewerWidget final : public FloatingWidgetContainer
{
    Q_OBJECT

  protected:
    enum class CurrentWidget : uint8_t {
        UNSET,
        IMAGE_VIEWER,
        VIDEO_PLAYER,
    };

  public:
    explicit ViewerWidget(QWidget *parent = nullptr);
    ~ViewerWidget() override;
    DELETE_COPY_MOVE_ROUTINES(ViewerWidget);

    void setInteractionEnabled(bool mode);
    bool showImage(std::unique_ptr<QPixmap> pixmap);
    bool showAnimation(QSharedPointer<QMovie> const &movie);
    void onScalingFinished(std::unique_ptr<QPixmap> scaled);
    void cleanVideoPlayer();

    ND QRect imageRect() const;
    ND qreal currentScale() const;
    ND QSize sourceSize() const;
    ND bool  interactionEnabled() const;
    ND bool  isDisplaying() const;
    ND bool  lockZoomEnabled() const;
    ND bool  lockViewEnabled() const;
    ND auto  scalingFilter() const -> ScalingFilter;

  protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool focusNextPrevChild(bool mode) override;

  private:
    void  enableImageViewer();
    void  enableVideoPlayer();
    void  disableImageViewer();
    void  disableVideoPlayer();

    ND QRect videoControlsArea() const;

  Q_SIGNALS:
    void scalingRequested(QSize, ScalingFilter);
    void zoomIn();
    void zoomOut();
    void zoomInCursor();
    void zoomOutCursor();
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void fitWindow();
    void fitWidth();
    void fitOriginal();
    void toggleTransparencyGrid();
    void draggedOut();
    void setFilterNearest();
    void setFilterBilinear();
    void setScalingFilter(ScalingFilter filter);
    void playbackFinished();
    void toggleLockZoom();
    void toggleLockView();
    void showScriptSettings();

  private Q_SLOTS:
    void onScaleChanged(qreal);
    void onVideoPlaybackFinished();
    void onAnimationPlaybackFinished();

  public Q_SLOTS:
    bool showVideo(QString const &file);
    void stopPlayback();
    void setFitMode(ImageFitMode mode);
    void closeImage();
    void hideCursor();
    void showCursor();
    void hideCursorTimed(bool restartTimer);

    ND ImageFitMode fitMode() const;

    // video control
    void pauseResumePlayback();
    void seek(int64_t pos);
    void seekRelative(int64_t pos);
    void seekBackward();
    void seekForward();
    void frameStep();
    void frameStepBack();
    void toggleMute();
    void volumeUp();
    void volumeDown();
    void startPlayback();
    void showContextMenu();
    void hideContextMenu();
    void showContextMenu(QPoint pos);
    void onFullscreenModeChanged(bool);
    void readSettings();
    void setLoopPlayback(bool mode);

  private:
    static constexpr int CURSOR_HIDE_TIMEOUT_MS = 1000;

    QVBoxLayout               *layout;
    ImageViewerV2             *imageViewer;
    VideoPlayerInitProxy      *videoPlayer;
    VideoControlsProxyWrapper *videoControls;
    ZoomIndicatorOverlayProxy *zoomIndicator;
    ContextMenu               *contextMenu;
    QTimer                     cursorTimer;

    CurrentWidget currentWidget = CurrentWidget::UNSET;

    bool mInteractionEnabled      = false;
    bool mWaylandCursorWorkaround = false;
    bool mIsFullscreen            = false;
};
