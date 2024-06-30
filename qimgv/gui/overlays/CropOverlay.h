#pragma once

#include "gui/customWidgets/FloatingWidget.h"
#include <QColor>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

class CropOverlay : public FloatingWidget
{
    Q_OBJECT

    enum class CursorAction : uint8_t {
        NO_DRAG          = 0,
        SELECTION_START  = 1,
        DRAG_SELECT      = 2,
        DRAG_MOVE        = 3,
        DRAG_LEFT        = 4,
        DRAG_RIGHT       = 5,
        DRAG_TOP         = 6,
        DRAG_BOTTOM      = 7,
        DRAG_TOPLEFT     = 8,
        DRAG_TOPRIGHT    = 9,
        DRAG_BOTTOMLEFT  = 10,
        DRAG_BOTTOMRIGHT = 11,
    };

  public:
    explicit CropOverlay(FloatingWidgetContainer *parent = nullptr);

    void setImageDrawRect(QRect);
    void setImageRealSize(QSize);
    void setButtonText(QString text);
    void setImageScale(qreal newScale);
    void clearSelection();

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    void recalculateGeometry() override;

    void drawSelection(QPainter *);
    void drawHandles(QBrush &, QPainter *);
    void updateHandlePositions();
    void prepareDrawElements();
    void resizeSelection(QPoint d);
    void resizeSelectionAR(QPoint d);
    void resizeSelectionFree(QPoint d);
    void updateSelectionDrawRect();
    void setCursorAction(CursorAction action);
    void setResizeAnchor(CursorAction action);

    QPoint setInsidePoint(QPoint, QRect);
    QRect  placeInside(QRect what, QRect where);

    ND auto mapPointToImage(QPoint p) const -> QPoint;
    ND auto hoverTarget(QPoint pos) const -> CursorAction;
    ND bool hasSelection() const;

  Q_SIGNALS:
    void positionChanged(float x, float y);
    void selectionChanged(QRect);
    void escPressed();
    void cropDefault();
    void cropSave();

  public Q_SLOTS:
    void hide();
    void onSelectionOutsideChange(QRect selection);
    void selectAll();
    void setAspectRatio(QPointF);
    void setLockAspectRatio(bool mode);

  private:
    QPoint startPos     = {0, 0};
    QPoint endPos       = {0, 0};
    QPoint moveStartPos = {0, 0};
    QPoint resizeAnchor = {0, 0};

    QRect imageRect            = {0, 0, 0, 0};
    QRect imageDrawRect        = {0, 0, 0, 0};
    QRect imageDrawRectDpi     = {0, 0, 0, 0};
    QRect selectionRect        = {0, 0, 0, 0};
    QRect selectionDrawRect    = {0, 0, 0, 0};
    QRect selectionDrawRectDpi = {0, 0, 0, 0};

    QRect  handles[8];
    QRectF handlesDpi[8];
    QBrush brushInactiveTint;
    QBrush brushDarkGray;
    QBrush brushGray;
    QBrush brushLightGray;
    QPen   selectionOutlinePen;

    // Temporary, for testing.
    QPointF ar    = {16, 9};
    qreal   scale = 1.0;

    qreal dpr;
    int   handleSize;
    bool  lockAspectRatio = false;

    CursorAction cursorAction = CursorAction::NO_DRAG;
};
