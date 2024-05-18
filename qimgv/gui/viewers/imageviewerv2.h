#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QElapsedTimer>
#include <QWheelEvent>
#include <QTimeLine>
#include <QScrollBar>
#include <QMovie>
#include <QColor>
#include <QTimer>
#include <QDebug>
#include <memory>
#include <cmath>
#include "settings.h"

#include "Common.h"

class ImageViewerV2 Q_DECL_FINAL : public QGraphicsView
{
    Q_OBJECT

  protected:
    enum class MouseInteractionState : uint8_t {
        NONE,
        DRAG_BEGIN,
        DRAG,
        PAN,
        ZOOM,
        WHEEL_ZOOM
    };
    
    enum class ViewLockMode : uint8_t {
        NONE,
        ZOOM,
        ALL
    };

  public:
    explicit ImageViewerV2(QWidget *parent = nullptr);
    ~ImageViewerV2() override = default;

    ImageViewerV2(ImageViewerV2 const &)            = delete;
    ImageViewerV2(ImageViewerV2 &&)                 = delete;
    ImageViewerV2 &operator=(ImageViewerV2 const &) = delete;
    ImageViewerV2 &operator=(ImageViewerV2 &&)      = delete;

    ND virtual auto fitMode() const -> ImageFitMode;
    ND virtual auto scaledRectR() const -> QRect;
    ND virtual auto currentScale() const -> qreal;
    ND virtual auto sourceSize() const -> QSize;
    ND virtual auto isDisplaying() const -> bool;
    ND virtual auto imageFits() const -> bool;
    ND virtual auto scalingFilter() const -> ScalingFilter;

    ND bool  scaledImageFits() const;
    ND bool  hasAnimation() const;
    ND QSize scaledSizeR() const;

    virtual void showImage(std::unique_ptr<QPixmap> pixmap_);
    virtual void showAnimation(std::shared_ptr<QMovie> const &movie_);
    virtual void setScaledPixmap(std::unique_ptr<QPixmap> newFrame);
    void         pauseResume();

    virtual QWidget *widget();

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
    virtual void setFitMode(ImageFitMode mode);
    virtual void setFitOriginal();
    virtual void setFitWidth();
    virtual void setFitWindow();
    virtual void zoomIn();
    virtual void zoomOut();
    virtual void zoomInCursor();
    virtual void zoomOutCursor();
    virtual void readSettings();
    virtual void scrollUp();
    virtual void scrollDown();
    virtual void scrollLeft();
    virtual void scrollRight();
    virtual void startAnimation();
    virtual void stopAnimation();
    virtual void closeImage();
    virtual void setExpandImage(bool mode);

    virtual void show();
    virtual void hide();

    virtual void setFilterNearest();
    virtual void setFilterBilinear();
    virtual void setScalingFilter(ScalingFilter filter);

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
    static constexpr int   ANIMATION_SPEED            = 150;
    static constexpr int   LARGE_VIEWPORT_SIZE        = 2'073'600;
    static constexpr int   SCROLL_UPDATE_RATE         = 7;
    static constexpr int   SCROLL_DISTANCE            = 220;
    static constexpr qreal SCROLL_SPEED_MILTIPLIER    = 1.3;
    static constexpr qreal TRACKPAD_SCROLL_MULTIPLIER = 0.7;
    static constexpr qreal FAST_SCALE_THRESHOLD       = 1.0;

    QGraphicsScene          *scene;
    std::shared_ptr<QPixmap> pixmap;
    std::unique_ptr<QPixmap> pixmapScaled;
    std::shared_ptr<QMovie>  movie;
    QGraphicsPixmapItem      pixmapItem;
    QGraphicsPixmapItem      pixmapItemScaled;

    QTimer     *animationTimer;
    QTimer     *scaleTimer;
    QScrollBar *hs;
    QScrollBar *vs;
    QPoint      mouseMoveStartPos;
    QPoint      mousePressPos;
    QPoint      drawPos;

    bool transparencyGrid     : 1;
    bool expandImage          : 1;
    bool smoothAnimatedImages : 1;
    bool smoothUpscaling      : 1;
    bool forceFastScale       : 1;
    bool keepFitMode          : 1;
    bool loopPlayback         : 1;
    bool mIsFullscreen        : 1;
    bool scrollBarWorkaround  : 1;
    bool useFixedZoomLevels   : 1;
    bool trackpadDetection    : 1;

    MouseInteractionState mouseInteraction;
    ImageFitMode          imageFitMode;
    ImageFitMode          imageFitModeDefault;
    ImageFocusPoint       focusIn1to1;
    ScalingFilter         mScalingFilter;
    ViewLockMode          mViewLock;

    // how many px you can move while holding RMB until it counts as a zoom attempt
    int   zoomThreshold = 4;
    int   dragThreshold = 10;
    qreal zoomStep      = 0.1;
    qreal dpr;
    qreal minScale;
    qreal maxScale;
    qreal fitWindowScale;
    qreal expandLimit;
    qreal lockedScale;

    QList<qreal>           zoomLevels;
    QPointF                savedViewportPos;
    QPair<QPointF, QPoint> zoomAnchor; // [pixmap coords, viewport coords]
    QElapsedTimer          lastTouchpadScroll;
    QPixmap               *checkboard;
    QTimeLine             *scrollTimeLineX;
    QTimeLine             *scrollTimeLineY;

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
    ND Qt::TransformationMode selectTransformationMode() const;

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

};
