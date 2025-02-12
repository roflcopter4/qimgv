/*
 * This widget combines ImageViewer / VideoPlayer.
 * Only one is displayed at a time.
 */

#include "ViewerWidget.h"

ViewerWidget::ViewerWidget(QWidget *parent)
    : FloatingWidgetContainer(parent),
      layout(new QVBoxLayout(this)),
      imageViewer(new ImageViewerV2(this)),
      videoPlayer(new VideoPlayerInitProxy(this)),
      videoControls(new VideoControlsProxyWrapper(this)),
      zoomIndicator(new ZoomIndicatorOverlayProxy(this)),
      clickZoneOverlay(new ClickZoneOverlay(this)),
      contextMenu(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

#ifdef Q_OS_LINUX
    // we cant check cursor position on wayland until the mouse is moved
    // use this to skip cursor check once
    if (qgetenv("XDG_SESSION_TYPE") == "wayland")
        mWaylandCursorWorkaround = true;
#endif
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(imageViewer);
    imageViewer->hide();

    connect(imageViewer, &ImageViewerV2::scalingRequested, this, &ViewerWidget::scalingRequested);
    connect(imageViewer, &ImageViewerV2::scaleChanged,     this, &ViewerWidget::onScaleChanged);
    connect(imageViewer, &ImageViewerV2::playbackFinished, this, &ViewerWidget::onAnimationPlaybackFinished);

    connect(this, &ViewerWidget::toggleTransparencyGrid, imageViewer, &ImageViewerV2::toggleTransparencyGrid);
    connect(this, &ViewerWidget::setFilterNearest,       imageViewer, &ImageViewerV2::setFilterNearest);
    connect(this, &ViewerWidget::setFilterBilinear,      imageViewer, &ImageViewerV2::setFilterBilinear);
    connect(this, &ViewerWidget::setScalingFilter,       imageViewer, &ImageViewerV2::setScalingFilter);

    layout->addWidget(videoPlayer);
    videoPlayer->hide();

    connect(videoPlayer,   &VideoPlayer::playbackFinished,           this, &ViewerWidget::onVideoPlaybackFinished);
    connect(videoControls, &VideoControlsProxyWrapper::seekBackward, this, &ViewerWidget::seekBackward);
    connect(videoControls, &VideoControlsProxyWrapper::seekForward,  this, &ViewerWidget::seekForward);
    connect(videoControls, &VideoControlsProxyWrapper::seek,         this, &ViewerWidget::seek);

    enableImageViewer();
    setInteractionEnabled(true);

    connect(&cursorTimer, &QTimer::timeout,           this, &ViewerWidget::hideCursor);
    connect(settings,     &Settings::settingsChanged, this, &ViewerWidget::readSettings);

    readSettings();
}

ViewerWidget::~ViewerWidget()
{
    delete imageViewer;
    delete videoPlayer;
    delete videoControls;
    delete zoomIndicator;
    delete clickZoneOverlay;
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

void ViewerWidget::cleanVideoPlayer()
{
    disableVideoPlayer();
    videoPlayer->hide();
}

void ViewerWidget::onScaleChanged(qreal scale)
{
    if (!isVisible())
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

void ViewerWidget::seek(int64_t pos)
{
    if (currentWidget == CurrentWidget::VIDEO_PLAYER) {
        videoPlayer->seek(pos);
    } else if (imageViewer->hasAnimation()) {
        imageViewer->stopAnimation();
        imageViewer->showAnimationFrame(pos);
    }
}

void ViewerWidget::seekRelative(int64_t pos)
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

// click zone input crutch
// --
// we can't process mouse events in the overlay
// cause they won't propagate to the ImageViewer, only to overlay's container (this widget)
// so we just grab them before they reach ImageViewer and do the needful
bool ViewerWidget::eventFilter(QObject *object, QEvent *event)
{
    // catch press and doubleclick
    // force doubleclick to act as press event for click zones
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        // disable feature for very small windows
        if (width() <= 250)
            return false;

        auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton || mouseEvent->modifiers()) {
            clickZoneOverlay->disableHighlight();
            return false;
        }
        if (clickZoneOverlay->leftZone().contains(mouseEvent->pos())) {
            clickZoneOverlay->setPressed(true);
            clickZoneOverlay->highlightLeft();
            imageViewer->disableDrags();
            actionManager->invokeAction(u"prevImage"_s);
            return true; // do not pass the event to imageViewer
        }
        if (clickZoneOverlay->rightZone().contains(mouseEvent->pos())) {
            clickZoneOverlay->setPressed(true);
            clickZoneOverlay->highlightRight();
            imageViewer->disableDrags();
            actionManager->invokeAction(u"nextImage"_s);
            return true;
        }
    }
    // right click produces QEvent::ContextMenu instead of QEvent::MouseButtonPress
    // this is NOT a QMouseEvent
    else if (event->type() == QEvent::ContextMenu) {
        clickZoneOverlay->disableHighlight();
        return false;
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        clickZoneOverlay->setPressed(false);
        imageViewer->enableDrags();
    }
    else if (event->type() == QEvent::MouseMove || event->type() == QEvent::Enter) {
        QPoint mousePos;
        if (event->type() == QEvent::MouseMove) {
            auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
            mousePos        = mouseEvent->pos();
            if (mouseEvent->buttons())
                return false;
        } else {
            auto enterEvent = dynamic_cast<QEnterEvent *>(event);
            mousePos        = enterEvent->pos();
        }
        if (clickZoneOverlay->leftZone().contains(mousePos)) {
            clickZoneOverlay->setPressed(false);
            clickZoneOverlay->highlightLeft();
            setCursor(Qt::PointingHandCursor);
            return true;
        }
        if (clickZoneOverlay->rightZone().contains(mousePos)) {
            clickZoneOverlay->setPressed(false);
            clickZoneOverlay->highlightRight();
            setCursor(Qt::PointingHandCursor);
            return true;
        }
        clickZoneOverlay->disableHighlight();
        setCursor(Qt::ArrowCursor);
    }
    else if (event->type() == QEvent::Leave) {
        clickZoneOverlay->disableHighlight();
        setCursor(Qt::ArrowCursor);
    }

    return false; // send event to imageViewer / videoplayer
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
            QPoint posMapped = mapFromGlobal(QCursor::pos());
            // Ignore when we are hovering the click zone
            if (clickZoneOverlay->leftZone().contains(posMapped) || clickZoneOverlay->leftZone().contains(posMapped))
                return;
            // Only hide when we are under viewer or player widget
            QWidget *w = qApp->widgetAt(QCursor::pos());
            if (w && (w == imageViewer->viewport() || w == videoPlayer->getPlayer().get())) {
                if (!videoControls->isVisible() || !videoControlsArea().contains(posMapped)) {
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
    setFocus();
    auto pos = QCursor::pos();
    showContextMenu(pos);
}

void ViewerWidget::showContextMenu(QPoint pos)
{
    if (isVisible() && interactionEnabled()) {
        if (!contextMenu) {
            contextMenu = new ContextMenu(this);
            connect(contextMenu, &ContextMenu::showScriptSettings, this, &ViewerWidget::showScriptSettings);
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
    if (settings->clickableEdges()) {
        imageViewer->viewport()->installEventFilter(this);
        videoPlayer->installEventFilter(this);
        clickZoneOverlay->show();
    } else {
        imageViewer->viewport()->removeEventFilter(this);
        videoPlayer->removeEventFilter(this);
        imageViewer->enableDrags();
        clickZoneOverlay->hide();
    }
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
        if (ShortcutBuilder::fromEvent(event) == u"Up"_sv && !actionManager->actionForShortcut(u"Up"_s).isEmpty())
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
