#pragma once

/* This class manages QGraphicsScene, ThumbnailWidget list,
 * scrolling, requesting and setting thumbnails.
 * It doesn't do actual positioning of thumbnails within the scene.
 * (But maybe it should?)
 *
 * Usage: subclass, implement layout-related stuff
 */

#include <QGraphicsView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QWheelEvent>
#include <QTimeLine>
#include <QTimer>
#include <QElapsedTimer>
#include <QScreen>

#include "gui/customwidgets/thumbnailwidget.h"
#include "gui/idirectoryview.h"
#include "shortcutbuilder.h"

#include "Common.h"

class ThumbnailView : public QGraphicsView, public IDirectoryView
{
    Q_OBJECT
    Q_INTERFACES(IDirectoryView)

  protected:
    enum class ThumbnailSelectMode : uint8_t {
        ACTIVATE_BY_PRESS,
        ACTIVATE_BY_DOUBLECLICK,
    };

    enum class ScrollDirection : uint8_t {
        FORWARDS,
        BACKWARDS,
    };

  public:
    explicit ThumbnailView(Qt::Orientation orientation, QWidget *parent = nullptr);

    void setDirectoryPath(QString path) override;
    void select(QList<int>) override;
    void select(int) override;

    ND auto selection() -> QList<int> & final { return mSelection; }
    ND auto selection() const -> QList<int> const & final { return mSelection; }
    ND auto itemCount() const -> qsizetype { return thumbnails.count(); }

    void setSelectMode(ThumbnailSelectMode mode);
    int  lastSelected();
    void clearSelection();
    void deselect(int index);

  public Q_SLOTS:
    void show();
    void resetViewport();
    void loadVisibleThumbnails();
    void loadVisibleThumbnailsDelayed();
    void addItem();

    ND int thumbnailSize() const;

    void showEvent(QShowEvent *event) override;
    void focusOnSelection() override = 0;
    void populate(int count) override;
    void setThumbnail(int pos, std::shared_ptr<Thumbnail> thumb) override;
    void insertItem(int index) override;
    void removeItem(int index) override;
    void reloadItem(int index) override;
    void setDragHover(int index) override;

  Q_SIGNALS:
    void itemActivated(int) override;
    void thumbnailsRequested(QList<int>, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(QList<int>) override;
    void draggedOver(int) override;
    void droppedInto(QMimeData const *, QObject *, int) override;

  private:
    void createScrollTimeLine();

    std::function<void(int)> centerOn;

    QTimer           loadTimer;
    QList<int>       mSelection;
    QPoint           dragStartPos;
    ThumbnailWidget *dragTarget;
    QElapsedTimer    scrollFrameTimer;
    QElapsedTimer    lastTouchpadScroll;
    Qt::Orientation  mOrientation = Qt::Horizontal;

    int  lastScrollFrameTime;
    int  mDrawScrollbarIndicator;
    bool blockThumbnailLoading;
    bool mCropThumbnails;
    bool mouseReleaseSelect;

    ThumbnailSelectMode selectMode;

  protected:
    ScrollDirection lastScrollDirection = ScrollDirection::FORWARDS;

    bool rangeSelection; // true if shift is pressed
    int  mThumbnailSize;
    int  offscreenPreloadArea = 3000;
    int  scrollRefreshRate    = 16;

    QList<ThumbnailWidget *> thumbnails;

    QScrollBar    *scrollBar;
    QTimeLine     *scrollTimeLine;
    QGraphicsScene scene;
    QPointF        viewportCenter;
    QList<int>  rangeSelectionSnapshot;
    QRect          indicator;

    static constexpr qreal SCROLL_ACCELERATION           = 1.4;
    static constexpr qreal SCROLL_MULTIPLIER             = 2.5;
    static constexpr int   SCROLL_ACCELERATION_THRESHOLD = 50;
    static constexpr int   SCROLL_DURATION               = 120;
    static constexpr int   indicatorSize                 = 2;
    static constexpr uint  LOAD_DELAY                    = 150;

    ND virtual auto createThumbnailWidget() -> ThumbnailWidget *   = 0;
    virtual void addItemToLayout(ThumbnailWidget *widget, int pos) = 0;
    virtual void removeItemFromLayout(int pos)                     = 0;
    virtual void removeAll()                                       = 0;
    virtual void updateScrollbarIndicator()                        = 0;
    virtual void updateLayout();
    virtual void fitSceneToContents();

    ND bool atSceneStart() const;
    ND bool atSceneEnd() const;
    ND bool checkRange(int pos) const;
    ND auto orientation() const -> Qt::Orientation;
    ND bool eventFilter(QObject *o, QEvent *ev) override;

    void setOrientation(Qt::Orientation orientation);
    void setCropThumbnails(bool);
    void setDrawScrollbarIndicator(bool mode);
    void addSelectionRange(int indexTo);
    void scrollToItem(int index);
    void scrollPrecise(int delta);
    void scrollByItem(int delta);
    void scrollSmooth(int delta);
    void scrollSmooth(int angleDelta, qreal multiplier, qreal acceleration);
    void scrollSmooth(int angleDelta, qreal multiplier, qreal acceleration, bool additive);
    void unloadAllThumbnails();

    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;
};
