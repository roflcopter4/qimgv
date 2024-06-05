/*
 * This widget combines ImageViewer / VideoPlayer.
 * Only one is displayed at a time.
 */

#include "viewerwidget.h"

ViewerWidget::ViewerWidget(QWidget *parent)
    : FloatingWidgetContainer(parent),
      imageViewer(nullptr),
      videoPlayer(nullptr),
      contextMenu(nullptr),
      videoControls(nullptr),
      currentWidget(CurrentWidget::UNSET),
      mInteractionEnabled(false),
      mWaylandCursorWorkaround(false),
      mIsFullscreen(false)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

#ifdef Q_OS_LINUX
    // we cant check cursor position on wayland until the mouse is moved
    // use this to skip cursor check once
    if (qgetenv("XDG_SESSION_TYPE") == "wayland")
        mWaylandCursorWorkaround = true;
#endif

    layout.setContentsMargins(0, 0, 0, 0);
    layout.setSpacing(0);
    this->setLayout(&layout);
    imageViewer = new ImageViewerV2(this);
    layout.addWidget(imageViewer);
    imageViewer->hide();

    connect(imageViewer, &ImageViewerV2::scalingRequested, this, &ViewerWidget::scalingRequested);
    connect(imageViewer, &ImageViewerV2::scaleChanged,     this, &ViewerWidget::onScaleChanged);
    connect(imageViewer, &ImageViewerV2::playbackFinished, this, &ViewerWidget::onAnimationPlaybackFinished);

    connect(this, &ViewerWidget::toggleTransparencyGrid, imageViewer, &ImageViewerV2::toggleTransparencyGrid);
    connect(this, &ViewerWidget::setFilterNearest,       imageViewer, &ImageViewerV2::setFilterNearest);
    connect(this, &ViewerWidget::setFilterBilinear,      imageViewer, &ImageViewerV2::setFilterBilinear);
    connect(this, &ViewerWidget::setScalingFilter,       imageViewer, &ImageViewerV2::setScalingFilter);

    videoPlayer = new VideoPlayerInitProxy(this);
    layout.addWidget(videoPlayer);
    videoPlayer->hide();
    videoControls = new VideoControlsProxyWrapper(this);
    // tmp no wrapper
    zoomIndicator = new ZoomIndicatorOverlayProxy(this);

    connect(videoPlayer, &VideoPlayer::playbackFinished, this, &ViewerWidget::onVideoPlaybackFinished);
    connect(videoControls, &VideoControlsProxyWrapper::seekBackward, this, &ViewerWidget::seekBackward);
    connect(videoControls, &VideoControlsProxyWrapper::seekForward,  this, &ViewerWidget::seekForward);
    connect(videoControls, &VideoControlsProxyWrapper::seek,         this, &ViewerWidget::seek);

    enableImageViewer();
    setInteractionEnabled(true);

    connect(&cursorTimer, &QTimer::timeout, this, &ViewerWidget::hideCursor);
    connect(settings, &Settings::settingsChanged, this, &ViewerWidget::readSettings);

    readSettings();
}

QRect ViewerWidget::imageRect() const
{
    return imageViewer && currentWidget == CurrentWidget::IMAGE_VIEWER
        ? imageViewer->scaledRectR()
        : QRect{0, 0, 0, 0};
}

qreal ViewerWidget::currentScale() const
{
    return imageViewer && currentWidget == CurrentWidget::IMAGE_VIEWER
        ? imageViewer->currentScale()
        : 1.0;
}

QSize ViewerWidget::sourceSize() const
{
    return imageViewer && currentWidget == CurrentWidget::IMAGE_VIEWER
        ? imageViewer->sourceSize()
        : QSize{0, 0};
}

// hide videoPlayer, show imageViewer
void ViewerWidget::enableImageViewer()
{
    if (currentWidget != CurrentWidget::IMAGE_VIEWER) {
        disableVideoPlayer();
        videoControls->setMode(PlaybackMode::ANIMATION);
        connect(imageViewer, &ImageViewerV2::durationChanged, videoControls, &VideoControlsProxyWrapper::setPlaybackDuration);
        connect(imageViewer, &ImageViewerV2::frameChanged,    videoControls, &VideoControlsProxyWrapper::setPlaybackPosition);
        connect(imageViewer, &ImageViewerV2::animationPaused, videoControls, &VideoControlsProxyWrapper::onPlaybackPaused);
        imageViewer->show();
        currentWidget = CurrentWidget::IMAGE_VIEWER;
    }
}

// hide imageViewer, show videoPlayer
void ViewerWidget::enableVideoPlayer()
{
    if (currentWidget != CurrentWidget::VIDEO_PLAYER) {
        disableImageViewer();
        videoControls->setMode(PlaybackMode::VIDEO);
        connect(videoPlayer, &VideoPlayer::durationChanged, videoControls, &VideoControlsProxyWrapper::setPlaybackDuration);
        connect(videoPlayer, &VideoPlayer::positionChanged, videoControls, &VideoControlsProxyWrapper::setPlaybackPosition);
        connect(videoPlayer, &VideoPlayer::videoPaused,     videoControls, &VideoControlsProxyWrapper::onPlaybackPaused);
        videoPlayer->show();
        currentWidget = CurrentWidget::VIDEO_PLAYER;
    }
}

void ViewerWidget::disableImageViewer()
{
    if (currentWidget == CurrentWidget::IMAGE_VIEWER) {
        currentWidget = CurrentWidget::UNSET;
        imageViewer->closeImage();
        imageViewer->hide();
        zoomIndicator->hide();
        disconnect(imageViewer, &ImageViewerV2::durationChanged, videoControls, &VideoControlsProxyWrapper::setPlaybackDuration);
        disconnect(imageViewer, &ImageViewerV2::frameChanged,    videoControls, &VideoControlsProxyWrapper::setPlaybackPosition);
        disconnect(imageViewer, &ImageViewerV2::animationPaused, videoControls, &VideoControlsProxyWrapper::onPlaybackPaused);
    }
}

void ViewerWidget::disableVideoPlayer()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER) {
        currentWidget = CurrentWidget::UNSET;
        // videoControls->hide();
        disconnect(videoPlayer, &VideoPlayer::durationChanged, videoControls, &VideoControlsProxyWrapper::setPlaybackDuration);
        disconnect(videoPlayer, &VideoPlayer::positionChanged, videoControls, &VideoControlsProxyWrapper::setPlaybackPosition);
        disconnect(videoPlayer, &VideoPlayer::videoPaused,     videoControls, &VideoControlsProxyWrapper::onPlaybackPaused);
        videoPlayer->setPaused(true);
        // even after calling hide() the player sends a few video frames
        // which paints over the imageviewer, causing corruption
        // so we do not HIDE it, but rather just cover it by imageviewer's widget
        // seems to work fine, might even feel a bit snappier
        if (!videoPlayer->isInitialized())
            videoPlayer->hide();
    }
}

void ViewerWidget::onScaleChanged(qreal scale)
{
    if (!this->isVisible())
        return;
    if (scale != 1.0) {
        zoomIndicator->setScale(scale);
        if (settings->zoomIndicatorMode() == ZoomIndicatorMode::ENABLED)
            zoomIndicator->show();
        else if ((settings->zoomIndicatorMode() == ZoomIndicatorMode::AUTO))
            zoomIndicator->show(1500);
    } else {
        zoomIndicator->hide();
    }
}

void ViewerWidget::onVideoPlaybackFinished()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        emit playbackFinished();
}

void ViewerWidget::onAnimationPlaybackFinished()
{
    if (currentWidget == CurrentWidget::IMAGE_VIEWER)
        emit playbackFinished();
}

void ViewerWidget::setInteractionEnabled(bool mode)
{
    if (mInteractionEnabled == mode)
        return;
    mInteractionEnabled = mode;
    if (mInteractionEnabled) {
        connect(this, &ViewerWidget::toggleLockZoom, imageViewer, &ImageViewerV2::toggleLockZoom);
        connect(this, &ViewerWidget::toggleLockView, imageViewer, &ImageViewerV2::toggleLockView);
        connect(this, &ViewerWidget::zoomIn,         imageViewer, &ImageViewerV2::zoomIn);
        connect(this, &ViewerWidget::zoomOut,        imageViewer, &ImageViewerV2::zoomOut);
        connect(this, &ViewerWidget::zoomInCursor,   imageViewer, &ImageViewerV2::zoomInCursor);
        connect(this, &ViewerWidget::zoomOutCursor,  imageViewer, &ImageViewerV2::zoomOutCursor);
        connect(this, &ViewerWidget::scrollUp,       imageViewer, &ImageViewerV2::scrollUp);
        connect(this, &ViewerWidget::scrollDown,     imageViewer, &ImageViewerV2::scrollDown);
        connect(this, &ViewerWidget::scrollLeft,     imageViewer, &ImageViewerV2::scrollLeft);
        connect(this, &ViewerWidget::scrollRight,    imageViewer, &ImageViewerV2::scrollRight);
        connect(this, &ViewerWidget::fitWindow,      imageViewer, &ImageViewerV2::setFitWindow);
        connect(this, &ViewerWidget::fitWidth,       imageViewer, &ImageViewerV2::setFitWidth);
        connect(this, &ViewerWidget::fitOriginal,    imageViewer, &ImageViewerV2::setFitOriginal);
        connect(imageViewer, &ImageViewerV2::draggedOut, this, &ViewerWidget::draggedOut);
        imageViewer->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    } else {
        disconnect(this, &ViewerWidget::zoomIn,        imageViewer, &ImageViewerV2::zoomIn);
        disconnect(this, &ViewerWidget::zoomOut,       imageViewer, &ImageViewerV2::zoomOut);
        disconnect(this, &ViewerWidget::zoomInCursor,  imageViewer, &ImageViewerV2::zoomInCursor);
        disconnect(this, &ViewerWidget::zoomOutCursor, imageViewer, &ImageViewerV2::zoomOutCursor);
        disconnect(this, &ViewerWidget::scrollUp,      imageViewer, &ImageViewerV2::scrollUp);
        disconnect(this, &ViewerWidget::scrollDown,    imageViewer, &ImageViewerV2::scrollDown);
        disconnect(this, &ViewerWidget::scrollLeft,    imageViewer, &ImageViewerV2::scrollLeft);
        disconnect(this, &ViewerWidget::scrollRight,   imageViewer, &ImageViewerV2::scrollRight);
        disconnect(this, &ViewerWidget::fitWindow,     imageViewer, &ImageViewerV2::setFitWindow);
        disconnect(this, &ViewerWidget::fitWidth,      imageViewer, &ImageViewerV2::setFitWidth);
        disconnect(this, &ViewerWidget::fitOriginal,   imageViewer, &ImageViewerV2::setFitOriginal);
        disconnect(imageViewer, &ImageViewerV2::draggedOut, this, &ViewerWidget::draggedOut);
        imageViewer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        hideContextMenu();
    }
}

bool ViewerWidget::interactionEnabled() const
{
    return mInteractionEnabled;
}

bool ViewerWidget::showImage(std::unique_ptr<QPixmap> pixmap)
{
    if (!pixmap)
        return false;
    stopPlayback();
    videoControls->hide();
    enableImageViewer();
    imageViewer->showImage(std::move(pixmap));
    hideCursorTimed(false);
    return true;
}

bool ViewerWidget::showAnimation(QSharedPointer<QMovie> const &movie)
{
    if (!movie)
        return false;
    stopPlayback();
    enableImageViewer();
    imageViewer->showAnimation(movie);
    hideCursorTimed(false);
    return true;
}

bool ViewerWidget::showVideo(QString const &file)
{
    stopPlayback();
    enableVideoPlayer();
    videoPlayer->showVideo(file);
    hideCursorTimed(false);
    return true;
}

void ViewerWidget::stopPlayback()
{
    if (currentWidget == CurrentWidget::IMAGE_VIEWER && imageViewer->hasAnimation())
        imageViewer->stopAnimation();
    if (currentWidget == CurrentWidget::VIDEO_PLAYER) {
        // stopping is visibly slower
        // videoPlayer->stop();
        videoPlayer->setPaused(true);
    }
}

void ViewerWidget::startPlayback()
{
    if (currentWidget == CurrentWidget::IMAGE_VIEWER && imageViewer->hasAnimation())
        imageViewer->startAnimation();
    if (currentWidget == CurrentWidget::VIDEO_PLAYER) {
        // stopping is visibly slower
        // videoPlayer->stop();
        videoPlayer->setPaused(false);
    }
}

void ViewerWidget::setFitMode(ImageFitMode mode)
{
    if (mode == ImageFitMode::WINDOW)
        emit fitWindow();
    else if (mode == ImageFitMode::WIDTH)
        emit fitWidth();
    else if (mode == ImageFitMode::ORIGINAL)
        emit fitOriginal();
}

ImageFitMode ViewerWidget::fitMode() const
{
    return imageViewer->fitMode();
}

void ViewerWidget::onScalingFinished(std::unique_ptr<QPixmap> scaled)
{
    imageViewer->setScaledPixmap(std::move(scaled));
}

void ViewerWidget::closeImage()
{
    imageViewer->closeImage();
    videoPlayer->stop();
    showCursor();
}

void ViewerWidget::pauseResumePlayback()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->pauseResume();
    else if (imageViewer->hasAnimation())
        imageViewer->pauseResume();
}

void ViewerWidget::seek(int pos)
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER) {
        videoPlayer->seek(pos);
    } else if (imageViewer->hasAnimation()) {
        imageViewer->stopAnimation();
        imageViewer->showAnimationFrame(pos);
    }
}

void ViewerWidget::seekRelative(int pos)
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->seekRelative(pos);
}

void ViewerWidget::seekBackward()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->seekRelative(-10);
}

void ViewerWidget::seekForward()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->seekRelative(10);
}

void ViewerWidget::frameStep()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->frameStep();
    else if (imageViewer->hasAnimation()) {
        imageViewer->stopAnimation();
        imageViewer->nextFrame();
    }
}

void ViewerWidget::frameStepBack()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->frameStepBack();
    else if (imageViewer->hasAnimation()) {
        imageViewer->stopAnimation();
        imageViewer->prevFrame();
    }
}

void ViewerWidget::toggleMute()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER) {
        videoPlayer->setMuted(!videoPlayer->muted());
        videoControls->onVideoMuted(videoPlayer->muted());
    }
}

void ViewerWidget::volumeUp()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->volumeUp();
}

void ViewerWidget::volumeDown()
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER)
        videoPlayer->volumeDown();
}

bool ViewerWidget::isDisplaying() const
{
    return (currentWidget == CurrentWidget::IMAGE_VIEWER && imageViewer->isDisplaying()) ||
           (currentWidget == CurrentWidget::VIDEO_PLAYER);
}

bool ViewerWidget::lockZoomEnabled() const
{
    return imageViewer->lockZoomEnabled();
}

bool ViewerWidget::lockViewEnabled() const
{
    return imageViewer->lockViewEnabled();
}

ScalingFilter ViewerWidget::scalingFilter() const
{
    return imageViewer->scalingFilter();
}

void ViewerWidget::mousePressEvent(QMouseEvent *event)
{
    hideContextMenu();
    event->ignore();
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    showCursor();
    hideCursorTimed(false);
    event->ignore();
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
    mWaylandCursorWorkaround = false;
    if (!(event->buttons() & Qt::LeftButton) && !(event->buttons() & Qt::RightButton)) {
        showCursor();
        hideCursorTimed(true);
    }
    if (currentWidget == CurrentWidget::VIDEO_PLAYER || imageViewer->hasAnimation()) {
        if (videoControlsArea().contains(event->pos()))
            videoControls->show();
        else
            videoControls->hide();
    }
    event->ignore();
}

void ViewerWidget::hideCursorTimed(bool restartTimer)
{
    if (restartTimer || !cursorTimer.isActive())
        cursorTimer.start(CURSOR_HIDE_TIMEOUT_MS);
}

void ViewerWidget::hideCursor()
{
    cursorTimer.stop();
    // ignore if we have something else open like settings window
    if (!isDisplaying() || !isActiveWindow())
        return;
    // ignore when menu is up
    if (contextMenu && contextMenu->isVisible())
        return;
    if (settings->cursorAutohide()) {
        // force hide on wayland until we can get the cursor pos
        if (mWaylandCursorWorkaround) {
            setCursor(QCursor(Qt::BlankCursor));
            videoControls->hide();
        } else {
            // only hide when we are under viewer or player widget
            QWidget *w = qApp->widgetAt(QCursor::pos());
            if (w && (w == imageViewer->viewport() || w == videoPlayer->getPlayer().get())) {
                if (!videoControls->isVisible() || !videoControlsArea().contains(mapFromGlobal(QCursor::pos()))) {
                    setCursor(QCursor(Qt::BlankCursor));
                    videoControls->hide();
                }
            }
        }
    }
}

QRect ViewerWidget::videoControlsArea() const
{
    QRect vcontrolsRect;
    if (settings->panelEnabled() && settings->panelPosition() == PanelPosition::BOTTOM)
        vcontrolsRect = QRect(0, 0, width(), 160); // inverted (top)
    else
        vcontrolsRect = QRect(0, height() - 160, width(), height());
    return vcontrolsRect;
}

void ViewerWidget::showCursor()
{
    cursorTimer.stop();
    if (cursor().shape() == Qt::BlankCursor)
        setCursor(QCursor(Qt::ArrowCursor));
}

void ViewerWidget::showContextMenu()
{
    this->setFocus();
    auto pos = QCursor::pos();
    showContextMenu(pos);
}

void ViewerWidget::showContextMenu(QPoint pos)
{
    if (isVisible() && interactionEnabled()) {
        if (!contextMenu) {
            contextMenu = std::make_unique<ContextMenu>(this);
            connect(contextMenu.get(), &ContextMenu::showScriptSettings, this, &ViewerWidget::showScriptSettings);
        }
        contextMenu->setImageEntriesEnabled(isDisplaying());
        if (!contextMenu->isVisible())
            contextMenu->showAt(pos);
        else
            contextMenu->hide();
    }
}

void ViewerWidget::onFullscreenModeChanged(bool mode)
{
    imageViewer->onFullscreenModeChanged(mode);
    mIsFullscreen = mode;
}

void ViewerWidget::readSettings()
{
    videoControls->onVideoMuted(!settings->playVideoSounds());
}

void ViewerWidget::setLoopPlayback(bool mode)
{
    imageViewer->setLoopPlayback(mode);
    videoPlayer->setLoopPlayback(mode);
}

void ViewerWidget::hideContextMenu()
{
    if (contextMenu)
        contextMenu->hide();
}

void ViewerWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    hideContextMenu();
}

// block native tab-switching so we can use it in shortcuts
bool ViewerWidget::focusNextPrevChild(bool)
{
    return false;
}

void ViewerWidget::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    if (currentWidget == CurrentWidget::VIDEO_PLAYER && event->key() == Qt::Key_Space) {
        videoPlayer->pauseResume();
        return;
    }
    if (currentWidget == CurrentWidget::IMAGE_VIEWER && imageViewer->isDisplaying()) {
        // switch to fitWidth via up arrow
        if (ShortcutBuilder::fromEvent(event) == QSV("Up") && !actionManager->actionForShortcut(QS("Up")).isEmpty())
        {
            if (imageViewer->fitMode() == ImageFitMode::WINDOW && imageViewer->scaledImageFits()) {
                imageViewer->setFitWidth();
                return;
            }
        }
    }
    actionManager->processEvent(event);
}

void ViewerWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    videoControls->hide();
}
