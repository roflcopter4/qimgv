#pragma once

#include "Settings.h"
#include <QColor>
#include <QDebug>
#include <QElapsedTimer>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMovie>
#include <QScrollBar>
#include <QTimeLine>
#include <QTimer>
#include <QWheelEvent>
#include <cmath>
#include <memory>

class ImageViewerV2 final : public QGraphicsView
{
    Q_OBJECT

  protected:
    enum class MouseInteractionState : uint8_t {
        NONE,
        DRAG_BEGIN,
        DRAG,
        PAN,
        ZOOM,
        WHEEL_ZOOM,
    };
    
    enum class ViewLockMode : uint8_t {
        NONE,
        ZOOM,
        ALL,
    };

  public:
    explicit ImageViewerV2(QWidget *parent = nullptr);
    ~ImageViewerV2() override;

    ImageViewerV2(ImageViewerV2 const &)            = delete;
    ImageViewerV2(ImageViewerV2 &&)                 = delete;
    ImageViewerV2 &operator=(ImageViewerV2 const &) = delete;
    ImageViewerV2 &operator=(ImageViewerV2 &&)      = delete;

    ND auto fitMode() const -> ImageFitMode;
    ND auto scaledRectR() const -> QRect;
    ND auto currentScale() const -> qreal;
    ND auto sourceSize() const -> QSize;
    ND auto isDisplaying() const -> bool;
    ND auto imageFits() const -> bool;
    ND auto scalingFilter() const -> ScalingFilter;
    ND bool scaledImageFits() const;
    ND bool hasAnimation() const;
    ND auto scaledSizeR() const -> QSize;
    ND auto widget() -> QWidget *;

    void showImage(std::unique_ptr<QPixmap> pixmap_);
    void showAnimation(QSharedPointer<QMovie> const &movie_);
    void setScaledPixmap(std::unique_ptr<QPixmap> newFrame);
    void pauseResume();

  Q_SIGNALS:
    void scalingRequested(QSize, ScalingFilter);
    void scaleChanged(qreal);
    void sourceSizeChanged(QSize);
    void imageAreaChanged(QRect);
    void draggedOut();
    void playbackFinished();
    void animationPaused(bool);
    void frameChanged(int);
    void durationChanged(int);

  public Q_SLOTS:
    void setFitMode(ImageFitMode mode);
    void setFitOriginal();
    void setFitWidth();
    void setFitWindow();
    void zoomIn();
    void zoomOut();
    void zoomInCursor();
    void zoomOutCursor();
    void readSettings();
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void startAnimation();
    void stopAnimation();
    void closeImage();
    void setExpandImage(bool mode);

    void show();
    void hide();

    void setFilterNearest();
    void setFilterBilinear();
    void setScalingFilter(ScalingFilter filter);

    void setLoopPlayback(bool mode);
    void toggleTransparencyGrid();

    void nextFrame();
    void prevFrame();

    bool showAnimationFrame(int frame);
    void onFullscreenModeChanged(bool mode);
    void toggleLockZoom();
    void toggleLockView();

    ND bool lockViewEnabled() const;
    ND bool lockZoomEnabled() const;

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent*event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent*event) override;
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

  protected Q_SLOTS:
    void onAnimationTimer();

  private Q_SLOTS:
    void requestScaling();
    void scrollToX(int x);
    void scrollToY(int y);
    void centerOnPixmap() const;
    void onScrollTimelineFinished();

  private:
    void zoomAnchored(qreal newScale);
    void fitNormal();
    void fitWidth();
    void fitWindow();
    void scroll(int dx, int dy, bool smooth);
    void mousePanWrapping(QMouseEvent *event);
    void mousePan(QMouseEvent const *event);
    void mouseMoveZoom(QMouseEvent const *event);
    void reset();
    void applyFitMode();
    void stopPosAnimation() const;

    ND QPointF sceneRoundPos(QPointF scenePoint) const;
    ND QRectF  sceneRoundRect(QRectF const &sceneRect) const;
    ND auto    selectTransformationMode() const -> Qt::TransformationMode;

    void doZoom(qreal newScale);
    void swapToOriginalPixmap();
    void setZoomAnchor(QPoint viewportPos);
    void updatePixmap(std::unique_ptr<QPixmap> newPixmap);
    void centerIfNecessary() const;
    void snapToEdges();
    void scrollSmooth(int dx, int dy);
    void scrollPrecise(int dx, int dy);
    void updateFitWindowScale();
    void updateMinScale();
    void fitFree(qreal scale);
    void applySavedViewportPos();
    void saveViewportPos();
    void lockZoom();
    void doZoomIn(bool atCursor);
    void doZoomOut(bool atCursor);

    static constexpr int   ANIMATION_SPEED            = 150;
    static constexpr int   LARGE_VIEWPORT_SIZE        = 2'073'600;
    static constexpr int   SCROLL_UPDATE_RATE         = 7;
    static constexpr int   SCROLL_DISTANCE            = 220;
    static constexpr qreal SCROLL_SPEED_MILTIPLIER    = 1.3;
    static constexpr qreal TRACKPAD_SCROLL_MULTIPLIER = 0.7;
    static constexpr qreal FAST_SCALE_THRESHOLD       = 1.0;

    QSharedPointer<QMovie>   movie;
    QSharedPointer<QPixmap>  pixmap;
    std::unique_ptr<QPixmap> pixmapScaled;
    QGraphicsPixmapItem      pixmapItem;
    QGraphicsPixmapItem      pixmapItemScaled;

    QGraphicsScene *scene;
    QPixmap        *checkboard;
    QTimer         *animationTimer;
    QTimer         *scaleTimer;
    QTimeLine      *scrollTimeLineX;
    QTimeLine      *scrollTimeLineY;

    QScrollBar *hs;
    QScrollBar *vs;
    qreal       dpr;

    QPoint mouseMoveStartPos;
    QPoint mousePressPos;
    QPoint drawPos;

    bool transparencyGrid     : 1 = false;
    bool expandImage          : 1 = false;
    bool smoothAnimatedImages : 1 = true;
    bool smoothUpscaling      : 1 = true;
    bool forceFastScale       : 1 = false;
    bool keepFitMode          : 1 = false;
    bool loopPlayback         : 1 = true;
    bool mIsFullscreen        : 1 = false;
    bool scrollBarWorkaround  : 1 = true;
    bool useFixedZoomLevels   : 1 = false;
    bool trackpadDetection    : 1 = true;

    MouseInteractionState mouseInteraction    = MouseInteractionState::NONE;
    ImageFitMode          imageFitMode        = ImageFitMode::WINDOW;
    ImageFitMode          imageFitModeDefault = ImageFitMode::WINDOW;
    ImageFocusPoint       focusIn1to1         = ImageFocusPoint::CENTER;
    ScalingFilter         mScalingFilter      = ScalingFilter::BILINEAR;
    ViewLockMode          mViewLock           = ViewLockMode::NONE;

    // how many px you can move while holding RMB until it counts as a zoom attempt
    int   zoomThreshold  = 4;
    int   dragThreshold  = 10;
    qreal zoomStep       = 0.1;
    qreal minScale       = 0.01;
    qreal maxScale       = 500.0;
    qreal fitWindowScale = 0.125;
    qreal expandLimit    = 0.0;
    qreal lockedScale    = 0.0;

    QList<qreal>           zoomLevels;
    QPair<QPointF, QPoint> zoomAnchor; // [pixmap coords, viewport coords]
    QPointF                savedViewportPos;
    QElapsedTimer          lastTouchpadScroll;
};
