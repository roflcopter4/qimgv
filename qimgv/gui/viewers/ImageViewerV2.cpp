#include "ImageViewerV2.h"

ImageViewerV2::ImageViewerV2(QWidget *parent)
    : QGraphicsView(parent),
      movie(nullptr),
      pixmap(nullptr),
      pixmapScaled(nullptr),
      scene(new QGraphicsScene(this)),
      checkboard(new QPixmap(u":res/icons/common/other/checkerboard.png"_s)),
      animationTimer(new QTimer(this)),
      scaleTimer(new QTimer(this)),
      scrollTimeLineX(new QTimeLine(1000, this)),
      scrollTimeLineY(new QTimeLine(1000, this)),
      hs(horizontalScrollBar()),
      vs(verticalScrollBar()),
      dpr(devicePixelRatioF()),
      zoomThreshold(static_cast<int>(devicePixelRatioF() * 4.0))
{
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptDrops(false);

    scrollTimeLineY->setEasingCurve(QEasingCurve::Type::OutSine);
    scrollTimeLineY->setDuration(ANIMATION_SPEED);
    scrollTimeLineY->setUpdateInterval(SCROLL_UPDATE_RATE);

    scrollTimeLineX->setEasingCurve(QEasingCurve::Type::OutSine);
    scrollTimeLineX->setDuration(ANIMATION_SPEED);
    scrollTimeLineX->setUpdateInterval(SCROLL_UPDATE_RATE);

    connect(scrollTimeLineX, &QTimeLine::finished, this, &ImageViewerV2::onScrollTimelineFinished);
    connect(scrollTimeLineY, &QTimeLine::finished, this, &ImageViewerV2::onScrollTimelineFinished);

    animationTimer->setSingleShot(true);
    scaleTimer->setSingleShot(true);
    scaleTimer->setInterval(80);
    lastTouchpadScroll.start();

    pixmapItem.setTransformationMode(Qt::TransformationMode::SmoothTransformation);
    pixmapItem.setScale(1.0);
    pixmapItem.setOffset(10000.0, 10000.0);
    pixmapItem.setTransformOriginPoint(10000.0, 10000.0);
    pixmapItemScaled.setScale(1.0);
    pixmapItemScaled.setOffset(10000.0, 10000.0);
    pixmapItemScaled.setTransformOriginPoint(10000.0, 10000.0);

    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    scene->setSceneRect(0.0, 0.0, 200000.0, 200000.0);
    scene->setBackgroundBrush(QColor(60, 60, 103));
    scene->addItem(&pixmapItem);
    scene->addItem(&pixmapItemScaled);
    pixmapItemScaled.hide();

    setFrameShape(QFrame::Shape::NoFrame);
    setScene(scene);

    connect(scrollTimeLineX, &QTimeLine::frameChanged, this, &ImageViewerV2::scrollToX);
    connect(scrollTimeLineY, &QTimeLine::frameChanged, this, &ImageViewerV2::scrollToY);
    connect(animationTimer,  &QTimer::timeout,         this, &ImageViewerV2::onAnimationTimer, Qt::UniqueConnection);
    connect(scaleTimer,      &QTimer::timeout,         this, &ImageViewerV2::requestScaling);

    readSettings();
    connect(settings, &Settings::settingsChanged, this, &ImageViewerV2::readSettings);
}

ImageViewerV2::~ImageViewerV2()
{
    delete checkboard;
}

void ImageViewerV2::readSettings()
{
    transparencyGrid     = settings->transparencyGrid();
    smoothAnimatedImages = settings->smoothAnimatedImages();
    smoothUpscaling      = settings->smoothUpscaling();
    expandImage          = settings->expandImage();
    expandLimit          = static_cast<qreal>(settings->expandLimit());
    if (expandLimit < 1.0)
        expandLimit = maxScale;
    keepFitMode         = settings->keepFitMode();
    imageFitModeDefault = settings->imageFitMode();
    zoomStep            = settings->zoomStep();
    focusIn1to1         = settings->focusPointIn1to1Mode();
    trackpadDetection   = settings->trackpadDetection();

    if ((useFixedZoomLevels = settings->useFixedZoomLevels())) {
        // zoomlevels are stored as a string, parse into list
        zoomLevels.clear();
        auto levelsStr = settings->zoomLevels().split(u',');
        for (auto const &i : levelsStr)
            zoomLevels.append(i.toDouble());
        std::ranges::sort(zoomLevels);
    }

    // set bg color
    onFullscreenModeChanged(mIsFullscreen);
    updateMinScale();
    setScalingFilter(settings->scalingFilter());
    setFitMode(imageFitModeDefault);
}

void ImageViewerV2::onFullscreenModeChanged(bool mode)
{
    QColor bgColor;
    mIsFullscreen = mode;
    if (mode) {
        bgColor = settings->colorScheme().background_fullscreen;
        bgColor.setAlphaF(1.0);
    } else {
        bgColor = settings->colorScheme().background;
        bgColor.setAlphaF(static_cast<float>(settings->backgroundOpacity()));
    }
    scene->setBackgroundBrush(bgColor);
}

void ImageViewerV2::startAnimation()
{
    if (movie && movie->frameCount() > 1) {
        stopAnimation();
        emit animationPaused(false);
        // movie->jumpToFrame(0);
        // emit frameChanged(0);
        animationTimer->start(movie->nextFrameDelay());
    }
}

void ImageViewerV2::stopAnimation()
{
    if (movie) {
        emit animationPaused(true);
        animationTimer->stop();
    }
}

void ImageViewerV2::pauseResume()
{
    if (movie) {
        if (animationTimer->isActive())
            stopAnimation();
        else
            startAnimation();
    }
}

void ImageViewerV2::onAnimationTimer()
{
    if (!movie)
        return;
    if (movie->currentFrameNumber() == movie->frameCount() - 1) {
        // last frame
        if (!loopPlayback) {
            emit animationPaused(true);
            emit playbackFinished();
            return;
        }
        movie->jumpToFrame(0);
    } else if (!movie->jumpToNextFrame()) {
        qDebug() << u"[Error] QMovie:" << movie->lastErrorString();
        stopAnimation();
        return;
    }

    emit frameChanged(movie->currentFrameNumber());
    updatePixmap(std::make_unique<QPixmap>(movie->currentPixmap()));
    animationTimer->start(movie->nextFrameDelay());
}

void ImageViewerV2::nextFrame()
{
    if (!movie)
        return;
    if (movie->currentFrameNumber() == movie->frameCount() - 1)
        showAnimationFrame(0);
    else
        showAnimationFrame(movie->currentFrameNumber() + 1);
}

void ImageViewerV2::prevFrame()
{
    if (!movie)
        return;
    if (movie->currentFrameNumber() == 0)
        showAnimationFrame(movie->frameCount() - 1);
    else
        showAnimationFrame(movie->currentFrameNumber() - 1);
}

bool ImageViewerV2::showAnimationFrame(int frame)
{
    if (!movie || frame < 0 || frame >= movie->frameCount())
        return false;
    if (movie->currentFrameNumber() == frame)
        return true;
    // at the first glance this may seem retarded
    // because it is
    // unfortunately i dont see a *better* way to do seeking with QMovie
    // QMovie::CacheAll is buggy and memory inefficient
    if (frame < movie->currentFrameNumber())
        movie->jumpToFrame(0);
    while (frame != movie->currentFrameNumber()) {
        if (!movie->jumpToNextFrame()) {
            qDebug() << u"[Error] QMovie:" << movie->lastErrorString();
            break;
        }
    }

    emit frameChanged(movie->currentFrameNumber());
    updatePixmap(std::make_unique<QPixmap>(movie->currentPixmap()));
    return true;
}

void ImageViewerV2::updatePixmap(std::unique_ptr<QPixmap> newPixmap)
{
    pixmap = QSharedPointer<QPixmap>(newPixmap.release());
    pixmap->setDevicePixelRatio(dpr);
    pixmapItem.setPixmap(*pixmap);
    pixmapItem.show();
    pixmapItem.update();
}

void ImageViewerV2::showAnimation(QSharedPointer<QMovie> const &movie_)
{
    if (movie_ && movie_->isValid()) {
        reset();
        movie = movie_;
        movie->jumpToFrame(0);
        auto mode = smoothAnimatedImages ? Qt::SmoothTransformation : Qt::FastTransformation;

        pixmapItem.setTransformationMode(mode);
        updatePixmap(std::make_unique<QPixmap>(movie->currentPixmap()));
        emit durationChanged(movie->frameCount());
        emit frameChanged(0);

        updateMinScale();
        if (!keepFitMode || imageFitMode == ImageFitMode::FREE)
            imageFitMode = imageFitModeDefault;

        if (mViewLock == ViewLockMode::NONE) {
            applyFitMode();
        } else {
            imageFitMode = ImageFitMode::FREE;
            fitFree(lockedScale);
            if (mViewLock == ViewLockMode::ALL)
                applySavedViewportPos();
        }
        startAnimation();
    }
}

// display & initialize
void ImageViewerV2::showImage(std::unique_ptr<QPixmap> pixmap_)
{
    reset();
    if (pixmap_) {
        pixmapItemScaled.hide();
        pixmap = QSharedPointer<QPixmap>(pixmap_.release());
        pixmap->setDevicePixelRatio(dpr);
        pixmapItem.setPixmap(*pixmap);
        auto mode = Qt::TransformationMode::SmoothTransformation;
        if (mScalingFilter == ScalingFilter::NEAREST)
            mode = Qt::TransformationMode::FastTransformation;
        pixmapItem.setTransformationMode(mode);
        pixmapItem.show();
        updateMinScale();

        if (!keepFitMode || imageFitMode == ImageFitMode::FREE)
            imageFitMode = imageFitModeDefault;

        if (mViewLock == ViewLockMode::NONE) {
            applyFitMode();
        } else {
            imageFitMode = ImageFitMode::FREE;
            fitFree(lockedScale);
            if (mViewLock == ViewLockMode::ALL)
                applySavedViewportPos();
        }
        requestScaling();
        update();
    }
}

// reset state, remove image & stop animation
void ImageViewerV2::reset()
{
    stopPosAnimation();
    pixmapItemScaled.setPixmap(QPixmap());
    pixmapScaled.reset(nullptr);
    pixmapItem.setPixmap(QPixmap());
    pixmapItem.setScale(1.0);
    pixmapItem.setOffset(10000, 10000);
    pixmap.reset();
    stopAnimation();
    movie = nullptr;
    centerOn(sceneRect().center());
    // when this view is not in focus this it won't update the background
    // so we force it here
    viewport()->update();
}

void ImageViewerV2::closeImage()
{
    reset();
}

void ImageViewerV2::setScaledPixmap(std::unique_ptr<QPixmap> newFrame)
{
    if (!movie && newFrame->size() != scaledSizeR() * dpr)
        return;

    pixmapScaled = std::move(newFrame);
    pixmapScaled->setDevicePixelRatio(dpr);
    pixmapItemScaled.setPixmap(*pixmapScaled);
    pixmapItem.hide();
    pixmapItemScaled.show();
}

bool ImageViewerV2::isDisplaying() const
{
    return pixmap != nullptr;
}

void ImageViewerV2::scrollUp()
{
    scroll(0, -SCROLL_DISTANCE, true);
}
void ImageViewerV2::scrollDown()
{
    scroll(0, SCROLL_DISTANCE, true);
}
void ImageViewerV2::scrollLeft()
{
    scroll(-SCROLL_DISTANCE, 0, true);
}
void ImageViewerV2::scrollRight()
{
    scroll(SCROLL_DISTANCE, 0, true);
}

// temporary override till application restart
void ImageViewerV2::toggleTransparencyGrid()
{
    transparencyGrid = !transparencyGrid;
    scene->update();
}

void ImageViewerV2::setScalingFilter(ScalingFilter filter)
{
    if (mScalingFilter == filter)
        return;
    mScalingFilter = filter;
    pixmapItem.setTransformationMode(selectTransformationMode());
    if (mScalingFilter == ScalingFilter::NEAREST)
        swapToOriginalPixmap();
    requestScaling();
}

void ImageViewerV2::setLoopPlayback(bool mode)
{
    if (movie && mode && loopPlayback != mode)
        startAnimation();
    loopPlayback = mode;
}

void ImageViewerV2::setFilterNearest()
{
    if (mScalingFilter != ScalingFilter::NEAREST) {
        mScalingFilter = ScalingFilter::NEAREST;
        pixmapItem.setTransformationMode(selectTransformationMode());
        swapToOriginalPixmap();
        requestScaling();
    }
}

void ImageViewerV2::setFilterBilinear()
{
    if (mScalingFilter != ScalingFilter::BILINEAR) {
        mScalingFilter = ScalingFilter::BILINEAR;
        pixmapItem.setTransformationMode(selectTransformationMode());
        requestScaling();
    }
}

// returns a mode based on current zoom level and a bunch of toggles
Qt::TransformationMode ImageViewerV2::selectTransformationMode() const
{
    Qt::TransformationMode mode = Qt::SmoothTransformation;
    if (forceFastScale) {
        mode = Qt::FastTransformation;
    } else if (movie) {
        if (!smoothAnimatedImages || (pixmapItem.scale() > 1.0 && !smoothUpscaling))
            mode = Qt::FastTransformation;
    } else {
        if ((pixmapItem.scale() > 1.0 && !smoothUpscaling) || mScalingFilter == ScalingFilter::NEAREST)
            mode = Qt::FastTransformation;
    }
    return mode;
}

void ImageViewerV2::setExpandImage(bool mode)
{
    expandImage = mode;
    updateMinScale();
    applyFitMode();
    requestScaling();
}

void ImageViewerV2::show()
{
    setMouseTracking(false);
    QGraphicsView::show();
    setMouseTracking(true);
}

void ImageViewerV2::hide()
{
    setMouseTracking(false);
    QWidget::hide();
}

void ImageViewerV2::requestScaling()
{
    if (!pixmap || pixmapItem.scale() == 1.0 || (!smoothUpscaling && pixmapItem.scale() >= 1.0) || movie)
        return;
    if (scaleTimer->isActive())
        scaleTimer->stop();
    // request "real" scaling when graphicsscene scaling is insufficient
    // (it uses a single pass bilinear which is sharp but produces artifacts on low zoom levels)
    if (currentScale() < FAST_SCALE_THRESHOLD)
        emit scalingRequested(scaledSizeR() * dpr, mScalingFilter);
}

bool ImageViewerV2::imageFits() const
{
    if (!pixmap)
        return true;
    return (pixmap->width() <= (viewport()->width() * devicePixelRatioF()) &&
            pixmap->height() <= (viewport()->height() * devicePixelRatioF()));
}

bool ImageViewerV2::scaledImageFits() const
{
    if (!pixmap)
        return true;
    QSize sz = scaledSizeR();
    return sz.width() <= viewport()->width() &&
           sz.height() <= viewport()->height();
}

ScalingFilter ImageViewerV2::scalingFilter() const
{
    return mScalingFilter;
}

QWidget *ImageViewerV2::widget()
{
    return this;
}

bool ImageViewerV2::hasAnimation() const
{
    return movie != nullptr;
}

//  Right button zooming / dragging logic
//  mouseMoveStartPos: stores the previous mouseMoveEvent() position,
//                     used to calculate delta.
//  mousePressPos: used to filter out accidental zoom events
//  mouseInteraction: tracks which action we are performing since the last mousePressEvent()
//
void ImageViewerV2::mousePressEvent(QMouseEvent *event)
{
    if (!pixmap) {
        QWidget::mousePressEvent(event); //NOLINT(bugprone-parent-virtual-call)
        return;
    }
    mouseMoveStartPos = event->pos();
    mousePressPos     = mouseMoveStartPos;
    if (event->button() & Qt::RightButton)
        setZoomAnchor(event->pos());
    else
        QWidget::mousePressEvent(event); //NOLINT(bugprone-parent-virtual-call)
}

void ImageViewerV2::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event); //NOLINT(bugprone-parent-virtual-call)

    if (!pixmap ||
        mouseInteraction == MouseInteractionState::DRAG ||
        mouseInteraction == MouseInteractionState::WHEEL_ZOOM)
    {
        return;
    }

    if (event->buttons() & Qt::LeftButton) {
        // ---------------- DRAG / PAN -------------------
        // select which action to start
        if (mouseInteraction == MouseInteractionState::NONE) {
            if (scaledImageFits()) {
                mouseInteraction = MouseInteractionState::DRAG_BEGIN;
            } else {
                mouseInteraction = MouseInteractionState::PAN;
                if (cursor().shape() != Qt::ClosedHandCursor)
                    setCursor(Qt::ClosedHandCursor);
            }
        }
        // emit a signal to start dnd; set flag to ignore further mouse move events
        if (mouseInteraction == MouseInteractionState::DRAG_BEGIN) {
            if (abs(mousePressPos.x() - event->pos().x()) > dragThreshold ||
                abs(mousePressPos.y() - event->pos().y()) > dragThreshold)
            {
                mouseInteraction = MouseInteractionState::NONE;
                emit draggedOut();
            }
        }
        // panning
        if (mouseInteraction == MouseInteractionState::PAN)
            mousePan(event);
    } else if (event->buttons() & Qt::RightButton) {
        // ------------------- ZOOM ----------------------
        // filter out possible mouse jitter by ignoring low delta drags
        if (mouseInteraction == MouseInteractionState::ZOOM ||
            abs(mousePressPos.y() - event->pos().y()) > zoomThreshold / dpr)
        {
            if (cursor().shape() != Qt::SizeVerCursor)
                setCursor(Qt::SizeVerCursor);
            mouseInteraction = MouseInteractionState::ZOOM;
            // avoid visible lags by forcing fast scale for large viewport sizes
            // this value possibly needs tweaking
            if (viewport()->width() * viewport()->height() > LARGE_VIEWPORT_SIZE)
                forceFastScale = true;
            mouseMoveZoom(event);
        }
    } else {
        event->ignore();
    }
}

void ImageViewerV2::mouseReleaseEvent(QMouseEvent *event)
{
    unsetCursor();
    if (forceFastScale) {
        forceFastScale = false;
        pixmapItem.setTransformationMode(selectTransformationMode());
    }
    if (!pixmap || mouseInteraction == MouseInteractionState::NONE) {
        QGraphicsView::mouseReleaseEvent(event);
        event->ignore();
    }
    mouseInteraction = MouseInteractionState::NONE;
}

// Warning for future me:
// For some reason in qgraphicsview wheelEvent is followed by moveEvent (wtf?)
void ImageViewerV2::wheelEvent(QWheelEvent *event)
{
    qDebug() << event->modifiers()
             << event->pixelDelta()
             << event->angleDelta()
             << lastTouchpadScroll.elapsed()
             << trackpadDetection;

#ifdef Q_OS_APPLE
    // this event goes off during force touch with Qt::ScrollPhase being set to begin/end
    // lets filter these
    if (event->phase() == Qt::ScrollBegin || event->phase() == Qt::ScrollEnd) {
        event->accept();
        return;
    }
#endif

    if (event->buttons() & Qt::RightButton) {
        event->accept();
        mouseInteraction = MouseInteractionState::WHEEL_ZOOM;
        int angleDelta   = event->angleDelta().ry();
        if (angleDelta > 0)
            zoomInCursor();
        else if (angleDelta < 0)
            zoomOutCursor();
    } else if (event->modifiers() == Qt::NoModifier) {
        QPoint pixelDelta = event->pixelDelta();
        QPoint angleDelta = event->angleDelta();
        /* for reference
         * linux
         *   trackpad:
         *     pixelDelta = (x,y) OR (0,0)
         *     angleDelta = (x*scale,y*scale) OR (x,y)
         *   wheel:
         *     pixelDelta = (0,0)     - libinput <= 1.18
         *     pixelDelta = (0,120*mtx) - libinput 1.19
         *     angleDelta = (0,120*mtx)
         * -----------------------------------------
         * macOS
         *   trackpad:
         *     pixelDelta = (x,y)
         *     angleDelta = (x*scale,y*scale)
         *   wheel:
         *     pixelDelta = (0,y*scrollAccel)
         *     angleDelta = (0,120*mtx)
         * -----------------------------------------
         * windows
         *   trackpad:
         *     ?? (dont have the hardware with precision drivers)
         *   wheel:
         *     pixelDelta = (0,0)
         *     AngleDelta = (0,120*mtx)
         */
        bool isWheel = true;
        if (trackpadDetection)
            isWheel = angleDelta.y() && !(angleDelta.y() % 120) && lastTouchpadScroll.elapsed() > 250;
        if (!isWheel) {
            lastTouchpadScroll.restart();
            event->accept();
            if (settings->imageScrolling() != ImageScrolling::NONE) {
                // scroll (high precision)
                stopPosAnimation();
                int dx = pixelDelta.x() ? pixelDelta.x() : angleDelta.x();
                int dy = pixelDelta.y() ? pixelDelta.y() : angleDelta.y();
                hs->setValue(hs->value() - dx * TRACKPAD_SCROLL_MULTIPLIER);
                vs->setValue(vs->value() - dy * TRACKPAD_SCROLL_MULTIPLIER);
                centerIfNecessary();
                snapToEdges();
            }
            qDebug() << u"trackpad";
        } else if (isWheel && settings->imageScrolling() == ImageScrolling::BY_TRACKPAD_AND_WHEEL) {
            // scroll by interval
            QRect imgRect = scaledRectR();
            // shift by 2px in case of img edge misalignment
            // todo: maybe even increase it to skip small distance scrolls?
            if ((event->angleDelta().y() < 0 && imgRect.bottom() > height() + 2) ||
                (event->angleDelta().y() > 0 && imgRect.top() < -2)) {
                event->accept();
                scroll(0, -angleDelta.y(), true);
            } else {
                qDebug() << u"pass1";
                event->ignore(); // not scrollable; passthrough event
            }
        } else {
            qDebug() << u"pass2";
            event->ignore();
            QWidget::wheelEvent(event); //NOLINT(bugprone-parent-virtual-call)
        }
        saveViewportPos();
    } else {
        qDebug() << u"pass3";
        event->ignore();
        QWidget::wheelEvent(event); //NOLINT(bugprone-parent-virtual-call)
    }
}

void ImageViewerV2::showEvent(QShowEvent *event)
{
    QGraphicsView::showEvent(event);
    // ensure we are properly resized
    qApp->processEvents();
    // reapply fitmode to fix viewport position
    if (imageFitMode == ImageFitMode::ORIGINAL)
        applyFitMode();
}

void ImageViewerV2::drawBackground(QPainter *painter, QRectF const &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    if (!isDisplaying() || !transparencyGrid || !pixmap->hasAlphaChannel())
        return;
    painter->drawTiledPixmap(pixmapItem.sceneBoundingRect(), *checkboard);
}

// simple pan behavior (cursor stops at the screen edges)
inline void ImageViewerV2::mousePan(QMouseEvent const *event)
{
    if (scaledImageFits())
        return;
    mouseMoveStartPos -= event->pos();
    scroll(mouseMoveStartPos.x(), mouseMoveStartPos.y(), false);
    mouseMoveStartPos = event->pos();
    saveViewportPos();
}

//  zooming while the right button is pressed
//  note: on reaching min zoom level the fitMode is set to ImageFitMode::WINDOW;
//        mid-zoom it is set to ImageFitMode::FREE.
//        ImageFitMode::FREE mode does not persist when changing images.
inline void ImageViewerV2::mouseMoveZoom(QMouseEvent const *event)
{
    constexpr qreal stepMultiplier = 0.003; // this one feels ok

    int   currentPos   = event->pos().y();
    int   moveDistance = mouseMoveStartPos.y() - currentPos;
    qreal newScale     = currentScale() * (1.0 + stepMultiplier * moveDistance * dpr);
    mouseMoveStartPos  = event->pos();
    imageFitMode       = ImageFitMode::FREE;

    zoomAnchored(newScale);
    centerIfNecessary();
    snapToEdges();
    if (pixmapItem.scale() == fitWindowScale)
        imageFitMode = ImageFitMode::WINDOW;
}

// scale at which current image fills the window
void ImageViewerV2::updateFitWindowScale()
{
    qreal scaleFitX = static_cast<qreal>(viewport()->width()) * devicePixelRatioF() / pixmap->width();
    qreal scaleFitY = static_cast<qreal>(viewport()->height()) * devicePixelRatioF() / pixmap->height();
    fitWindowScale = scaleFitX < scaleFitY ? scaleFitX : scaleFitY;
    if (expandImage && fitWindowScale > expandLimit)
        fitWindowScale = expandLimit;
}

void ImageViewerV2::updateMinScale()
{
    if (!pixmap)
        return;
    updateFitWindowScale();
    if (settings->unlockMinZoom())
        if (!pixmap->isNull())
            minScale = qMax(10.0 / pixmap->width(), 10.0 / pixmap->height());
        else
            minScale = 1.0;
    else if (imageFits())
        minScale = 1.0;
    else
        minScale = fitWindowScale;
    if (mViewLock != ViewLockMode::NONE && lockedScale < minScale)
        minScale = lockedScale;
}

void ImageViewerV2::fitWidth()
{
    if (!pixmap)
        return;
    auto scaleX = static_cast<qreal>(viewport()->width()) * devicePixelRatioF() / pixmap->width();

    if (!expandImage && scaleX > 1.0)
        scaleX = 1.0;
    if (scaleX > expandLimit)
        scaleX = expandLimit;
    if (currentScale() != scaleX) {
        swapToOriginalPixmap();
        doZoom(scaleX);
    }
    centerIfNecessary();
    // just center somewhere at the top then do snap
    if (scaledSizeR().height() > viewport()->height()) {
        QPointF centerTarget = mapToScene(viewport()->rect()).boundingRect().center();
        centerTarget.setY(0);
        centerOn(centerTarget);
    }
    snapToEdges();
}

void ImageViewerV2::fitWindow()
{
    if (!pixmap)
        return;
    if (imageFits() && !expandImage) {
        fitNormal();
    } else {
        if (currentScale() != fitWindowScale) {
            swapToOriginalPixmap();
            doZoom(fitWindowScale);
        }
        // There's either a qt bug or I am misusing something.
        // First call to scrollbar->setValue() produces wrong results
        // - unless when called from eventloop
        if (scrollBarWorkaround) {
            scrollBarWorkaround = false;
            QTimer::singleShot(0, this, SLOT(centerOnPixmap()));
        } else {
            centerOnPixmap();
        }
    }
}

void ImageViewerV2::fitNormal()
{
    fitFree(1.0);
}

void ImageViewerV2::fitFree(qreal scale)
{
    if (!pixmap)
        return;
    if (focusIn1to1 == ImageFocusPoint::TOP) {
        doZoom(scale);
        centerIfNecessary();
        if (scaledSizeR().height() > viewport()->height()) {
            QPointF centerTarget = sceneRect().center();
            centerTarget.setY(0);
            centerOn(centerTarget);
        }
        snapToEdges();
    } else {
        if (focusIn1to1 == ImageFocusPoint::CENTER)
            setZoomAnchor(viewport()->rect().center());
        else
            setZoomAnchor(mapFromGlobal(QCursor::pos()));
        zoomAnchored(scale);
        centerIfNecessary();
        snapToEdges();
    }
}

void ImageViewerV2::applyFitMode()
{
    switch (imageFitMode) {
    case ImageFitMode::ORIGINAL: fitNormal(); break;
    case ImageFitMode::WIDTH:    fitWidth();  break;
    case ImageFitMode::WINDOW:   fitWindow(); break;
    case ImageFitMode::FREE:     break;
    }
}

// public, sends scale request
void ImageViewerV2::setFitMode(ImageFitMode mode)
{
    if (scaleTimer->isActive())
        scaleTimer->stop();
    stopPosAnimation();
    imageFitMode = mode;
    applyFitMode();
    requestScaling();
}

// public, sends scale request
void ImageViewerV2::setFitOriginal()
{
    setFitMode(ImageFitMode::ORIGINAL);
}

// public, sends scale request
void ImageViewerV2::setFitWidth()
{
    setFitMode(ImageFitMode::WIDTH);
    requestScaling();
}

// public, sends scale request
void ImageViewerV2::setFitWindow()
{
    setFitMode(ImageFitMode::WINDOW);
    requestScaling();
}

void ImageViewerV2::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // reset this so we won't generate unnecessary drag'n'drop event
    mousePressPos = mapFromGlobal(QCursor::pos());
    // Qt emits some unnecessary resizeEvents on startup
    // so we try to ignore them
    if (parentWidget()->isVisible()) {
        stopPosAnimation();
        updateMinScale();
        if (imageFitMode == ImageFitMode::FREE || imageFitMode == ImageFitMode::ORIGINAL) {
            centerIfNecessary();
            snapToEdges();
        } else {
            applyFitMode();
        }
        update();
        if (scaleTimer->isActive())
            scaleTimer->stop();
        scaleTimer->start();
        saveViewportPos();
    }
}

void ImageViewerV2::centerOnPixmap() const
{
    auto imgRect = pixmapItem.sceneBoundingRect();
    auto vport   = mapToScene(viewport()->geometry()).boundingRect();
    hs->setValue(static_cast<int>(pixmapItem.offset().x() - (vport.width() - imgRect.width()) / 2));
    vs->setValue(static_cast<int>(pixmapItem.offset().y() - (vport.height() - imgRect.height()) / 2));
}

void ImageViewerV2::stopPosAnimation() const
{
    if (scrollTimeLineX->state() == QTimeLine::Running)
        scrollTimeLineX->stop();
    if (scrollTimeLineY->state() == QTimeLine::Running)
        scrollTimeLineY->stop();
}

inline void ImageViewerV2::scroll(int dx, int dy, bool smooth)
{
    if (smooth)
        scrollSmooth(dx, dy);
    else
        scrollPrecise(dx, dy);
}

void ImageViewerV2::mousePanWrapping(QMouseEvent *)
{
    (void)this;
}

void ImageViewerV2::scrollSmooth(int dx, int dy)
{
    if (dx) {
        int  delta       = dx < 0 ? SCROLL_DISTANCE : -SCROLL_DISTANCE;
        bool redirect    = false;
        int  currentXPos = hs->value();
        int  newEndFrame = currentXPos - delta;

        if ((newEndFrame < currentXPos && currentXPos < scrollTimeLineX->endFrame()) ||
            (newEndFrame > currentXPos && currentXPos > scrollTimeLineX->endFrame()))
        {
            redirect = true;
        }
        if (scrollTimeLineX->state() == QTimeLine::Running) {
            int oldEndFrame = scrollTimeLineX->endFrame();
            // if(oldEndFrame == currentYPos)
            //     createScrollTimeLine();
            if (!redirect)
                newEndFrame = oldEndFrame - static_cast<int>(delta * SCROLL_SPEED_MILTIPLIER);
        }
        scrollTimeLineX->stop();
        scrollTimeLineX->setFrameRange(currentXPos, newEndFrame);
        scrollTimeLineX->start();
    }
    if (dy) {
        int  delta       = dy < 0 ? SCROLL_DISTANCE : -SCROLL_DISTANCE;
        bool redirect    = false;
        int  currentYPos = vs->value();
        int  newEndFrame = currentYPos - delta;

        if ((newEndFrame < currentYPos && currentYPos < scrollTimeLineY->endFrame()) ||
            (newEndFrame > currentYPos && currentYPos > scrollTimeLineY->endFrame()))
        {
            redirect = true;
        }
        if (scrollTimeLineY->state() == QTimeLine::Running) {
            int oldEndFrame = scrollTimeLineY->endFrame();
            // if(oldEndFrame == currentYPos)
            //     createScrollTimeLine();
            if (!redirect)
                newEndFrame = oldEndFrame - static_cast<int>(delta * SCROLL_SPEED_MILTIPLIER);
        }
        scrollTimeLineY->stop();
        scrollTimeLineY->setFrameRange(currentYPos, newEndFrame);
        scrollTimeLineY->start();
    }
    saveViewportPos();
}

void ImageViewerV2::scrollPrecise(int dx, int dy)
{
    stopPosAnimation();
    hs->setValue(hs->value() + dx);
    vs->setValue(vs->value() + dy);
    centerIfNecessary();
    snapToEdges();
    saveViewportPos();
}

// used by scrollTimeLine
void ImageViewerV2::scrollToX(int x)
{
    hs->setValue(x);
    centerIfNecessary();
    snapToEdges();
    update();
    qApp->processEvents();
}

// used by scrollTimeLine
void ImageViewerV2::scrollToY(int y)
{
    vs->setValue(y);
    centerIfNecessary();
    snapToEdges();
    update();
    qApp->processEvents();
}

void ImageViewerV2::onScrollTimelineFinished()
{
    saveViewportPos();
}

void ImageViewerV2::swapToOriginalPixmap()
{
    if (!pixmap || !pixmapItemScaled.isVisible())
        return;
    pixmapItemScaled.hide();
    pixmapItemScaled.setPixmap(QPixmap());
    pixmapScaled.reset(nullptr);
    pixmapItem.show();
}

void ImageViewerV2::setZoomAnchor(QPoint viewportPos)
{
    zoomAnchor = QPair<QPointF, QPoint>(pixmapItem.mapFromScene(mapToScene(viewportPos)), viewportPos);
}

void ImageViewerV2::zoomAnchored(qreal newScale)
{
    if (currentScale() != newScale) {
        QPointF vportCenter = mapToScene(viewport()->geometry()).boundingRect().center();
        doZoom(newScale);
        // calculate shift to adjust viewport center
        // we do this in viewport coordinates to avoid any rounding errors
        QPointF diff = zoomAnchor.second - mapFromScene(pixmapItem.mapToScene(zoomAnchor.first));
        centerOn(vportCenter - diff);
        requestScaling();
    }
}

// zoom in around viewport center
void ImageViewerV2::zoomIn()
{
    doZoomIn(false);
}

// zoom in around cursor if its inside window
void ImageViewerV2::zoomInCursor()
{
    doZoomIn(true);
}

void ImageViewerV2::doZoomIn(bool atCursor)
{
    if (atCursor && underMouse())
        setZoomAnchor(mapFromGlobal(QCursor::pos()));
    else
        setZoomAnchor(viewport()->rect().center());
    qreal newScale = currentScale() * (1.0 + zoomStep);
    if (useFixedZoomLevels && zoomLevels.count()) {
        if (currentScale() < zoomLevels.first()) {
            newScale = qMin(currentScale() * (1.0 + zoomStep), zoomLevels.first());
        } else if (currentScale() >= zoomLevels.last()) {
            newScale = currentScale() * (1.0 + zoomStep);
        } else {
            for (qsizetype i = 0; i < zoomLevels.count(); i++) {
                qreal level = zoomLevels.at(i);
                if (currentScale() < level) {
                    newScale = level;
                    break;
                }
            }
        }
    }
    zoomAnchored(newScale);
    centerIfNecessary();
    snapToEdges();
    imageFitMode = ImageFitMode::FREE;
    if (pixmapItem.scale() == fitWindowScale)
        imageFitMode = ImageFitMode::WINDOW;
}

// zoom out around viewport center
void ImageViewerV2::zoomOut()
{
    doZoomOut(false);
}

// zoom out around cursor if its inside window
void ImageViewerV2::zoomOutCursor()
{
    doZoomOut(true);
}

void ImageViewerV2::doZoomOut(bool atCursor)
{
    if (atCursor && underMouse())
        setZoomAnchor(mapFromGlobal(QCursor::pos()));
    else
        setZoomAnchor(viewport()->rect().center());

    qreal newScale = currentScale() * (1.0 - zoomStep);
    if (useFixedZoomLevels && zoomLevels.count()) {
        if (currentScale() > zoomLevels.last()) {
            newScale = qMax(zoomLevels.last(), currentScale() * (1.0 - zoomStep));
        } else if (currentScale() <= zoomLevels.first()) {
            newScale = currentScale() * (1.0 - zoomStep);
        } else {
            for (qsizetype i = zoomLevels.count() - 1; i >= 0; --i) {
                qreal level = zoomLevels.at(i);
                if (currentScale() > level) {
                    newScale = level;
                    break;
                }
            }
        }
    }
    zoomAnchored(newScale);
    centerIfNecessary();
    snapToEdges();
    imageFitMode = ImageFitMode::FREE;
    if (pixmapItem.scale() == fitWindowScale)
        imageFitMode = ImageFitMode::WINDOW;
}

void ImageViewerV2::toggleLockZoom()
{
    if (!isDisplaying())
        return;
    if (mViewLock != ViewLockMode::ZOOM) {
        mViewLock = ViewLockMode::ZOOM;
        lockZoom();
    } else {
        mViewLock = ViewLockMode::NONE;
    }
}

bool ImageViewerV2::lockZoomEnabled() const
{
    return mViewLock == ViewLockMode::ZOOM;
}

void ImageViewerV2::lockZoom()
{
    lockedScale  = pixmapItem.scale();
    imageFitMode = ImageFitMode::FREE;
    saveViewportPos();
}

void ImageViewerV2::toggleLockView()
{
    if (!isDisplaying())
        return;
    if (mViewLock != ViewLockMode::ALL) {
        mViewLock = ViewLockMode::ALL;
        lockZoom();
        saveViewportPos();
    } else {
        mViewLock = ViewLockMode::NONE;
    }
}

bool ImageViewerV2::lockViewEnabled() const
{
    return mViewLock == ViewLockMode::ALL;
}

// savedViewportPos is [0...1][0...1]
// values are where viewport center is on the image
void ImageViewerV2::saveViewportPos()
{
    if (mViewLock != ViewLockMode::ALL)
        return;

    QGraphicsPixmapItem *item = &pixmapItem;
    QPointF sceneCenter = mapToScene(viewport()->rect().center()) + QPointF(1, 1);
    QRectF  itemRect    = item->sceneBoundingRect();

    savedViewportPos.setX(qBound(0.0, (sceneCenter.x() - itemRect.left()) / itemRect.width(), 1.0));
    savedViewportPos.setY(qBound(0.0, (sceneCenter.y() - itemRect.top()) / itemRect.height(), 1.0));
}

void ImageViewerV2::applySavedViewportPos()
{
    QGraphicsPixmapItem *item = &pixmapItem;
    QRectF  itemRect = item->sceneBoundingRect();
    QPointF newScenePos;
    newScenePos.setX(itemRect.left() + itemRect.width() * savedViewportPos.x());
    newScenePos.setY(itemRect.top() + itemRect.height() * savedViewportPos.y());
    centerOn(newScenePos);
    centerIfNecessary();
    snapToEdges();
}

void ImageViewerV2::centerIfNecessary() const
{
    if (!pixmap)
        return;
    QSize sz      = scaledSizeR();
    auto  imgRect = pixmapItem.sceneBoundingRect();
    auto  vport   = mapToScene(viewport()->geometry()).boundingRect();
    if (sz.width() <= viewport()->width())
        hs->setValue(static_cast<int>(pixmapItem.offset().x() - (vport.width() - imgRect.width()) / 2.0));
    if (sz.height() <= viewport()->height())
        vs->setValue(static_cast<int>(pixmapItem.offset().y() - (vport.height() - imgRect.height()) / 2.0));
}

void ImageViewerV2::snapToEdges()
{
    QRect imgRect = scaledRectR();
    // current vport center
    QPointF centerTarget = mapToScene(viewport()->rect()).boundingRect().center();
    qreal   xShift       = 0;
    qreal   yShift       = 0;
    if (imgRect.width() > width()) {
        if (imgRect.left() > 0)
            xShift = imgRect.left();
        else if (imgRect.right() < width())
            xShift = imgRect.right() - width();
    }
    if (imgRect.height() > height()) {
        if (imgRect.top() > 0)
            yShift = imgRect.top();
        else if (imgRect.bottom() < height())
            yShift = imgRect.bottom() - height();
    }
    centerOn(centerTarget + QPointF(xShift, yShift));
}

void ImageViewerV2::doZoom(qreal newScale)
{
    if (!pixmap)
        return;
    newScale = qBound(minScale, newScale, 500.0);
    // fix scene position to integer values
    auto tl = pixmapItem.sceneBoundingRect().topLeft().toPoint();
    pixmapItem.setOffset(tl);
    pixmapItem.setScale(newScale);

    pixmapItem.setTransformationMode(selectTransformationMode());
    swapToOriginalPixmap();
    emit scaleChanged(newScale);
}

ImageFitMode ImageViewerV2::fitMode() const
{
    return imageFitMode;
}

// rounds a point in scene coordinates so it stays on the same spot on viewport
QPointF ImageViewerV2::sceneRoundPos(QPointF scenePoint) const
{
    return mapToScene(mapFromScene(scenePoint));
}

// rounds a rect in scene coordinates so it stays on the same spot on viewport
// the result is what's actually drawn on screen (incl. size)
QRectF ImageViewerV2::sceneRoundRect(QRectF const &sceneRect) const
{
    return {sceneRoundPos(sceneRect.topLeft()), sceneRect.size()};
}

// size as it appears on screen (rounded)
QSize ImageViewerV2::scaledSizeR() const
{
    if (!pixmap)
        return {0, 0};
    QRectF pixmapSceneRect = pixmapItem.mapRectToScene(pixmapItem.boundingRect());
    return sceneRoundRect(pixmapSceneRect).size().toSize();
}

// in viewport coords (rounded up)
QRect ImageViewerV2::scaledRectR() const
{
    QRectF pixmapSceneRect = pixmapItem.mapRectToScene(pixmapItem.boundingRect());
    return {mapFromScene(pixmapSceneRect.topLeft()),
            mapFromScene(pixmapSceneRect.bottomRight())};
}

qreal ImageViewerV2::currentScale() const
{
    return pixmapItem.scale();
}

QSize ImageViewerV2::sourceSize() const
{
    if (!pixmap)
        return {0, 0};
    return pixmap->size();
}
