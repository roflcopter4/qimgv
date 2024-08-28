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
    DELETE_COPY_MOVE_ROUTINES(ThumbnailView);

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

  private Q_SLOTS:
    void onValueChanged();

  Q_SIGNALS:
    void itemActivated(qsizetype) override;
    void thumbnailsRequested(SelectionList, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(SelectionList) override;
    void draggedOver(qsizetype) override;
    void droppedInto(QMimeData const *, QObject *, qsizetype) override;

  protected:
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
    void scrollSmooth(int angleDelta);
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
    void createScrollTimeLine();
    void onScrollTimeLineFrameChanged(int value);
    void onScrollTimeLineFinished();

    static constexpr qreal SCROLL_ACCELERATION           = 1.4;
    static constexpr qreal SCROLL_MULTIPLIER             = 2.5;
    static constexpr int   SCROLL_ACCELERATION_THRESHOLD = 50;
    static constexpr int   SCROLL_DURATION               = 120;
    static constexpr int   LOAD_DELAY                    = 150;

    std::function<void(int)> centerOn;

    ThumbnailWidget *dragTarget = nullptr;
    QElapsedTimer    scrollFrameTimer;
    QElapsedTimer    lastTouchpadScroll;
    SelectionList    mSelection;
    QTimer           loadTimer;
    QPoint           dragStartPos;

    qint64 lastScrollFrameTime     = 0;
    int    mDrawScrollbarIndicator = true;
    quint8 mOrientation            = Qt::Horizontal;
    bool   blockThumbnailLoading   = false;
    bool   mCropThumbnails         = false;
    bool   mouseReleaseSelect      = false;

    ThumbnailSelectMode selectMode = ThumbnailSelectMode::ACTIVATE_BY_PRESS;

  protected:
    ScrollDirection lastScrollDirection = ScrollDirection::FORWARDS;

    bool rangeSelection       = false; // true if shift is pressed
    int  mThumbnailSize       = 120;
    int  offscreenPreloadArea = 3000;
    int  scrollRefreshRate    = 16;

    QPointF         viewportCenter;
    QRect           indicator;
    QGraphicsScene *scene;
    QScrollBar     *scrollBar      = nullptr;
    QTimeLine      *scrollTimeLine = nullptr;
    SelectionList   rangeSelectionSnapshot;

    QList<ThumbnailWidget *> thumbnails;
};
