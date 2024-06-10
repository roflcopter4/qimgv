#pragma once

/* This class manages QGraphicsScene, ThumbnailWidget list,
 * scrolling, requesting and setting thumbnails.
 * It doesn't do actual positioning of thumbnails within the scene.
 * (But maybe it should?)
 *
 * Usage: subclass, implement layout-related stuff
 */

#include "gui/IDirectoryView.h"
#include "gui/customWidgets/ThumbnailWidget.h"
#include "ShortcutBuilder.h"
#include <QElapsedTimer>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QScreen>
#include <QScrollBar>
#include <QTimeLine>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

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
    explicit ThumbnailView(Qt::Orientation orientation, QWidget *parent);
    ~ThumbnailView() override;

    Q_DISABLE_COPY_MOVE(ThumbnailView)

    void setDirectoryPath(QString path) override;
    void select(SelectionList) override;
    void select(qsizetype) override;

    ND SelectionList       &selection()       final { return mSelection; }
    ND SelectionList const &selection() const final { return mSelection; }

    ND qsizetype itemCount() const { return thumbnails.count(); }
    ND qsizetype lastSelected() const;

    void setSelectMode(ThumbnailSelectMode mode);
    void clearSelection();
    void deselect(qsizetype index);

  public Q_SLOTS:
    void show();
    void resetViewport();
    void loadVisibleThumbnails();
    void loadVisibleThumbnailsDelayed();
    void addItem();

    int thumbnailSize() const;

    void showEvent(QShowEvent *event) override;
    void focusOnSelection() override = 0;
    void populate(qsizetype count) override;
    void setThumbnail(qsizetype pos, QSharedPointer<Thumbnail> thumb) override;
    void insertItem(qsizetype index) override;
    void removeItem(qsizetype index) override;
    void reloadItem(qsizetype index) override;
    void setDragHover(qsizetype index) override;

  Q_SIGNALS:
    void itemActivated(qsizetype) override;
    void thumbnailsRequested(SelectionList, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(SelectionList) override;
    void draggedOver(qsizetype) override;
    void droppedInto(QMimeData const *, QObject *, qsizetype) override;

  private:
    void createScrollTimeLine();
    void onScrollTimeLineFrameChanged(int value);
    void onScrollTimeLineFinished();

    std::function<void(int)> centerOn;

    QTimer           loadTimer;
    SelectionList     mSelection;
    QPoint           dragStartPos;
    ThumbnailWidget *dragTarget;
    QElapsedTimer    scrollFrameTimer;
    QElapsedTimer    lastTouchpadScroll;
    qint64            lastScrollFrameTime;
    Qt::Orientation  mOrientation = Qt::Horizontal;

    int    mDrawScrollbarIndicator;
    bool   blockThumbnailLoading;
    bool   mCropThumbnails;
    bool   mouseReleaseSelect;

    ThumbnailSelectMode selectMode;

  protected:
    int  mThumbnailSize       = 120;
    int  offscreenPreloadArea = 3000;
    int  scrollRefreshRate    = 16;
    bool rangeSelection; // true if shift is pressed
    ScrollDirection lastScrollDirection = ScrollDirection::FORWARDS;

    QPointF       viewportCenter;
    QRect         indicator;
    SelectionList rangeSelectionSnapshot;
    QList<ThumbnailWidget *> thumbnails;

    QScrollBar     *scrollBar;
    QTimeLine      *scrollTimeLine;
    QGraphicsScene *scene;

    ND virtual auto createThumbnailWidget() -> ThumbnailWidget *         = 0;
    virtual void addItemToLayout(ThumbnailWidget *widget, qsizetype pos) = 0;
    virtual void removeItemFromLayout(qsizetype pos)                     = 0;
    virtual void removeAll()                                             = 0;
    virtual void updateScrollbarIndicator()                              = 0;

    virtual void updateLayout();
    virtual void fitSceneToContents();

    ND bool atSceneStart() const;
    ND bool atSceneEnd() const;
    ND bool checkRange(qsizetype pos) const;
    ND auto orientation() const -> Qt::Orientation;
    ND bool eventFilter(QObject *o, QEvent *ev) override;

    void setOrientation(Qt::Orientation orientation);
    void setCropThumbnails(bool);
    void setDrawScrollbarIndicator(bool mode);
    void addSelectionRange(qsizetype indexTo);
    void scrollToItem(qsizetype index);
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

    static constexpr qreal indicatorSize = 2.0;

  private:
    static constexpr qreal SCROLL_ACCELERATION           = 1.4;
    static constexpr qreal SCROLL_MULTIPLIER             = 2.5;
    static constexpr int   SCROLL_ACCELERATION_THRESHOLD = 50;
    static constexpr int   SCROLL_DURATION               = 120;
    static constexpr int   LOAD_DELAY                    = 150;
};
