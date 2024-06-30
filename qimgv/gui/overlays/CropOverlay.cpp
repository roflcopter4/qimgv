#include "CropOverlay.h"

// TODO: this is pretty old. Clean up

CropOverlay::CropOverlay(FloatingWidgetContainer *parent)
    : FloatingWidget(parent),
      dpr(devicePixelRatio()),
      handleSize(static_cast<int>(8.0 * dpr))
{
    setMouseTracking(true);
    prepareDrawElements();

    brushInactiveTint.setColor(QColor(0, 0, 0, 160));
    brushInactiveTint.setStyle(Qt::SolidPattern);
    brushDarkGray.setColor(QColor(120, 120, 120, 160));
    brushDarkGray.setStyle(Qt::SolidPattern);
    brushGray.setColor(QColor(150, 150, 150, 160));
    brushGray.setStyle(Qt::SolidPattern);
    brushLightGray.setColor(QColor(220, 220, 220, 160));
    brushLightGray.setStyle(Qt::SolidPattern);
    selectionOutlinePen.setColor(Qt::white);
    selectionOutlinePen.setStyle(Qt::SolidLine);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);

    if (parent)
        setContainerSize(parent->size());
    hide();
}

//------------------------------------------------------------------------------
void CropOverlay::prepareDrawElements()
{
    for (auto &handle : handles)
        handle = QRect(0, 0, handleSize * 2, handleSize * 2);
}

//------------------------------------------------------------------------------
void CropOverlay::setImageRealSize(QSize sz)
{
    imageRect.setSize(sz);
    clearSelection();
}

//------------------------------------------------------------------------------
void CropOverlay::setImageDrawRect(QRect _imageDrawRect)
{
    imageDrawRect.setTopLeft(_imageDrawRect.topLeft() * dpr);
    imageDrawRect.setBottomRight(_imageDrawRect.bottomRight() * dpr);
    imageDrawRectDpi = _imageDrawRect;
}

//------------------------------------------------------------------------------
void CropOverlay::setImageScale(qreal newScale)
{
    scale = newScale;
}

//------------------------------------------------------------------------------
void CropOverlay::clearSelection()
{
    if (hasSelection()) {
        startPos          = QPoint(0, 0);
        endPos            = QPoint(0, 0);
        selectionRect     = QRect(0, 0, 0, 0);
        selectionDrawRect = QRect(0, 0, 0, 0);
        update();
        emit selectionChanged(selectionRect);
    }
}

//------------------------------------------------------------------------------
bool CropOverlay::hasSelection() const
{
    return selectionRect.width() && selectionRect.height();
}

//------------------------------------------------------------------------------
void CropOverlay::selectAll()
{
    selectionRect = imageRect;
    updateSelectionDrawRect();
    updateHandlePositions();
    update();
    emit selectionChanged(selectionRect);
}

//------------------------------------------------------------------------------
void CropOverlay::setLockAspectRatio(bool mode)
{
    lockAspectRatio = mode;
}

//------------------------------------------------------------------------------
void CropOverlay::setAspectRatio(QPointF ratio)
{
    if (ratio.x() == 0.0 || ratio.y() == 0.0)
        return;
    ar = ratio;
    setLockAspectRatio(true);
    // force resize selection area
    if (hasSelection()) {
        resizeSelection(QPoint(0, 0));
        update();
        emit selectionChanged(selectionRect);
    }
}

//------------------------------------------------------------------------------
void CropOverlay::hide()
{
    startPos = QPoint(0, 0);
    endPos   = QPoint(0, 0);
    imageDrawRect.setRect(0, 0, 0, 0);
    clearSelection();
    FloatingWidget::hide();
}

//------------------------------------------------------------------------------
void CropOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    if (!hasSelection()) {
        p.setPen(Qt::NoPen);
        p.setBrush(brushInactiveTint);
        p.drawRect(imageDrawRectDpi);
    } else {
        // draw tint over non-selected area of the image
        QRegion tintRegion(imageDrawRectDpi);
        p.setClipRegion(tintRegion.subtracted(selectionDrawRectDpi));
        p.fillRect(imageDrawRectDpi, brushInactiveTint);
        p.setClipRegion(rect());
        // selection outline & handles
        if (selectionDrawRect.width() > 0 && selectionDrawRect.height() > 0) {
            drawSelection(&p);
            // draw handles if there is no interaction going on
            // and selection is large enough
            if (cursorAction == CursorAction::NO_DRAG && selectionDrawRect.width() >= 90 && selectionDrawRect.height() >= 90)
                drawHandles(brushGray, &p);
        }
    }
}

//------------------------------------------------------------------------------
void CropOverlay::drawSelection(QPainter *painter)
{
    painter->save();
    painter->setPen(selectionOutlinePen);
    painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
    painter->drawRect(selectionDrawRectDpi);
    painter->restore();
}

//------------------------------------------------------------------------------
void CropOverlay::drawHandles(QBrush &brush, QPainter *painter)
{
    painter->save();
    painter->setBrush(brush);
    painter->setPen(selectionOutlinePen);
    for (int i = 0; i < 8; i++)
        painter->drawRect(handlesDpi[i]);
    painter->restore();
}

//------------------------------------------------------------------------------
// moves first QRect inside second
// resizes inner rect to outer size if needed
QRect CropOverlay::placeInside(QRect inner, QRect outer)
{
    if (inner.width() > outer.width()) {
        inner.setLeft(outer.left());
        inner.setRight(outer.right());
    } else {
        if (inner.left() < outer.left())
            inner.moveLeft(outer.left());
        if (inner.right() > outer.right())
            inner.moveRight(outer.right());
    }
    if (inner.height() > outer.height()) {
        inner.setTop(outer.top());
        inner.setBottom(outer.bottom());
    } else {
        if (inner.top() < outer.top())
            inner.moveTop(outer.top());
        if (inner.bottom() > outer.bottom())
            inner.moveBottom(outer.bottom());
    }
    return inner;
}

//------------------------------------------------------------------------------
CropOverlay::CursorAction CropOverlay::hoverTarget(QPoint pos) const
{
    if (handles[0].contains(pos))
        return CursorAction::DRAG_TOPLEFT;
    if (handles[1].contains(pos))
        return CursorAction::DRAG_TOPRIGHT;
    if (handles[2].contains(pos))
        return CursorAction::DRAG_BOTTOMLEFT;
    if (handles[3].contains(pos))
        return CursorAction::DRAG_BOTTOMRIGHT;
    if (handles[4].contains(pos))
        return CursorAction::DRAG_LEFT;
    if (handles[5].contains(pos))
        return CursorAction::DRAG_RIGHT;
    if (handles[6].contains(pos))
        return CursorAction::DRAG_TOP;
    if (handles[7].contains(pos))
        return CursorAction::DRAG_BOTTOM;
    if (selectionDrawRect.contains(pos))
        return CursorAction::DRAG_MOVE;
    return CursorAction::NO_DRAG;
}

//------------------------------------------------------------------------------
void CropOverlay::setCursorAction(CursorAction action)
{
    switch (action) {
    case CursorAction::DRAG_TOPLEFT:     setCursor({Qt::SizeFDiagCursor}); break;
    case CursorAction::DRAG_TOPRIGHT:    setCursor({Qt::SizeBDiagCursor}); break;
    case CursorAction::DRAG_BOTTOMLEFT:  setCursor({Qt::SizeBDiagCursor}); break;
    case CursorAction::DRAG_BOTTOMRIGHT: setCursor({Qt::SizeFDiagCursor}); break;
    case CursorAction::DRAG_LEFT:        setCursor({Qt::SizeHorCursor});   break;
    case CursorAction::DRAG_RIGHT:       setCursor({Qt::SizeHorCursor});   break;
    case CursorAction::DRAG_TOP:         setCursor({Qt::SizeVerCursor});   break;
    case CursorAction::DRAG_BOTTOM:      setCursor({Qt::SizeVerCursor});   break;
    case CursorAction::DRAG_MOVE:        setCursor({Qt::OpenHandCursor});  break;
    default:                             setCursor({Qt::ArrowCursor});     break;
    }
}

//------------------------------------------------------------------------------
void CropOverlay::setResizeAnchor(CursorAction action)
{
    switch (action) {
    case CursorAction::DRAG_TOPLEFT:     resizeAnchor = selectionRect.bottomRight(); break;
    case CursorAction::DRAG_TOPRIGHT:    resizeAnchor = selectionRect.bottomLeft();  break;
    case CursorAction::DRAG_BOTTOMLEFT:  resizeAnchor = selectionRect.topRight();    break;
    case CursorAction::DRAG_BOTTOMRIGHT: resizeAnchor = selectionRect.topLeft();     break;
    case CursorAction::DRAG_LEFT:        resizeAnchor = selectionRect.topRight();    break;
    case CursorAction::DRAG_RIGHT:       resizeAnchor = selectionRect.bottomLeft();  break;
    case CursorAction::DRAG_TOP:         resizeAnchor = selectionRect.bottomLeft();  break;
    case CursorAction::DRAG_BOTTOM:      resizeAnchor = selectionRect.topLeft();     break;
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// TODO: flip selection rectangle
void CropOverlay::resizeSelection(QPoint delta)
{
    if (lockAspectRatio)
        resizeSelectionAR(delta);
    else
        resizeSelectionFree(delta);
    updateSelectionDrawRect();
    updateHandlePositions();
}

//------------------------------------------------------------------------------
// TODO: split this up
void CropOverlay::resizeSelectionAR(QPoint delta)
{
    QSizeF newSz(selectionRect.size());
    QSizeF maxSz;
    /* ~JustQtThings~
     * int QRect::bottom() const
     *   Note that for historical reasons this function returns top() + height() - 1;
     *   use y() + height() to retrieve the true y-coordinate.
     * same for QRect::right()
     */
    switch (cursorAction) {
    case CursorAction::DRAG_TOPLEFT:
        // get max selection size
        maxSz.setWidth(selectionRect.right() + 1);
        maxSz.setHeight(selectionRect.bottom() + 1);
        // apply ar to current size
        newSz.setHeight(newSz.width() / ar.x() * ar.y());
        // scale to fit
        newSz.scale(qMin(newSz.width() - delta.x(), maxSz.width()), maxSz.height(), Qt::KeepAspectRatio);
        // apply
        selectionRect.setSize(newSz.toSize());
        // move the corner so it stays in original place
        selectionRect.moveBottomRight(resizeAnchor);
        break;
    case CursorAction::DRAG_TOPRIGHT:
        maxSz.setWidth(imageRect.width() - selectionRect.left());
        maxSz.setHeight(selectionRect.bottom() + 1);
        newSz.setHeight(newSz.width() / ar.x() * ar.y());
        newSz.scale(qMin(newSz.width() + delta.x(), maxSz.width()), maxSz.height(), Qt::KeepAspectRatio);
        selectionRect.setSize(newSz.toSize());
        selectionRect.moveBottomLeft(resizeAnchor);
        break;
    case CursorAction::DRAG_LEFT:
    case CursorAction::DRAG_BOTTOMLEFT:
        maxSz.setWidth(selectionRect.right() + 1);
        maxSz.setHeight(imageRect.height() - selectionRect.top());
        newSz.setHeight(newSz.width() / ar.x() * ar.y());
        newSz.scale(qMin(newSz.width() - delta.x(), maxSz.width()), maxSz.height(), Qt::KeepAspectRatio);
        selectionRect.setSize(newSz.toSize());
        selectionRect.moveTopRight(resizeAnchor);
        break;
    case CursorAction::DRAG_TOP:
        maxSz.setWidth(imageRect.width() - selectionRect.left());
        maxSz.setHeight(selectionRect.bottom() + 1);
        newSz.setHeight(newSz.width() / ar.x() * ar.y());
        newSz.scale(maxSz.width(), qMin(newSz.height() - delta.y(), maxSz.height()), Qt::KeepAspectRatio);
        selectionRect.setSize(newSz.toSize());
        selectionRect.moveBottomLeft(resizeAnchor);
        break;
    case CursorAction::DRAG_BOTTOM:
        maxSz.setWidth(imageRect.width() - selectionRect.left());
        maxSz.setHeight(imageRect.height() - selectionRect.top());
        newSz.setHeight(newSz.width() / ar.x() * ar.y());
        newSz.scale(maxSz.width(), qMin(newSz.height() + delta.y(), maxSz.height()), Qt::KeepAspectRatio);
        selectionRect.setSize(newSz.toSize());
        break;
    case CursorAction::DRAG_RIGHT:
    case CursorAction::DRAG_BOTTOMRIGHT:
    default:
        maxSz.setWidth(imageRect.width() - selectionRect.left());
        maxSz.setHeight(imageRect.height() - selectionRect.top());
        newSz.setHeight(newSz.width() / ar.x() * ar.y());
        newSz.scale(qMin(newSz.width() + delta.x(), maxSz.width()), maxSz.height(), Qt::KeepAspectRatio);
        selectionRect.setSize(newSz.toSize());
        break;
    }
}

//------------------------------------------------------------------------------
void CropOverlay::resizeSelectionFree(QPoint delta)
{
    // resize
    switch (cursorAction) {
    case CursorAction::DRAG_TOPLEFT:     selectionRect.setTopLeft(selectionRect.topLeft() + delta);         break;
    case CursorAction::DRAG_TOPRIGHT:    selectionRect.setTopRight(selectionRect.topRight() + delta);       break;
    case CursorAction::DRAG_BOTTOMLEFT:  selectionRect.setBottomLeft(selectionRect.bottomLeft() + delta);   break;
    case CursorAction::DRAG_BOTTOMRIGHT: selectionRect.setBottomRight(selectionRect.bottomRight() + delta); break;
    case CursorAction::DRAG_LEFT:        selectionRect.setLeft(selectionRect.left() + delta.x());           break;
    case CursorAction::DRAG_RIGHT:       selectionRect.setRight(selectionRect.right() + delta.x());         break;
    case CursorAction::DRAG_TOP:         selectionRect.setTop(selectionRect.top() + delta.y());             break;
    case CursorAction::DRAG_BOTTOM:      selectionRect.setBottom(selectionRect.bottom() + delta.y());       break;
    default:
        break;
    }

    // Flip if needed
    if (selectionRect.width() < 0) {
        int left = selectionRect.left();
        selectionRect.setLeft(selectionRect.right());
        selectionRect.setRight(left);
        switch (cursorAction) {
        case CursorAction::DRAG_TOPLEFT:     cursorAction = CursorAction::DRAG_TOPRIGHT;    break;
        case CursorAction::DRAG_TOPRIGHT:    cursorAction = CursorAction::DRAG_TOPLEFT;     break;
        case CursorAction::DRAG_BOTTOMLEFT:  cursorAction = CursorAction::DRAG_BOTTOMRIGHT; break;
        case CursorAction::DRAG_BOTTOMRIGHT: cursorAction = CursorAction::DRAG_BOTTOMLEFT;  break;
        case CursorAction::DRAG_LEFT:        cursorAction = CursorAction::DRAG_RIGHT;       break;
        case CursorAction::DRAG_RIGHT:       cursorAction = CursorAction::DRAG_LEFT;        break;
        default:
            break;
        }
        setCursorAction(cursorAction);
    }

    if (selectionRect.height() < 0) {
        int top = selectionRect.top();
        selectionRect.setTop(selectionRect.bottom());
        selectionRect.setBottom(top);
        switch (cursorAction) {
        case CursorAction::DRAG_TOPLEFT:     cursorAction = CursorAction::DRAG_BOTTOMLEFT;  break;
        case CursorAction::DRAG_TOPRIGHT:    cursorAction = CursorAction::DRAG_BOTTOMRIGHT; break;
        case CursorAction::DRAG_BOTTOMLEFT:  cursorAction = CursorAction::DRAG_TOPLEFT;     break;
        case CursorAction::DRAG_BOTTOMRIGHT: cursorAction = CursorAction::DRAG_TOPRIGHT;    break;
        case CursorAction::DRAG_TOP:         cursorAction = CursorAction::DRAG_BOTTOM;      break;
        case CursorAction::DRAG_BOTTOM:      cursorAction = CursorAction::DRAG_TOP;         break;
        default:
            break;
        }
        setCursorAction(cursorAction);
    }

    // Crop selection so it's bounded by imageRect
    selectionRect = selectionRect.intersected(imageRect);
}

//------------------------------------------------------------------------------
// generate a draw rectangle from selectionRect
void CropOverlay::updateSelectionDrawRect()
{
    selectionDrawRect = selectionRect;
    selectionDrawRect.moveTopLeft(selectionRect.topLeft() * scale + imageDrawRect.topLeft());
    selectionDrawRect.setSize((selectionRect.size() - QSize(1, 1)) * scale);
    selectionDrawRectDpi = QRect(QPoint(selectionDrawRect.topLeft() / dpr), selectionDrawRect.size() / dpr);
}

//------------------------------------------------------------------------------
// map a point to image coordinate
QPoint CropOverlay::mapPointToImage(QPoint p) const
{
    // shift relative to (0,0)
    if (p.x() < imageDrawRect.x())
        p.setX(imageDrawRect.x());
    if (p.y() < imageDrawRect.y())
        p.setY(imageDrawRect.y());
    p.setX(p.x() - imageDrawRect.x());
    p.setY(p.y() - imageDrawRect.y());
    p = p / scale;
    // bound to image size
    if (p.x() > imageRect.width())
        p.setX(imageRect.width() - 1);
    if (p.y() > imageRect.height())
        p.setY(imageRect.height() - 1);
    return p;
}

//------------------------------------------------------------------------------
void CropOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        cursorAction = hoverTarget(event->pos() * dpr);
        setCursorAction(cursorAction);
        setResizeAnchor(cursorAction);
        moveStartPos = event->pos();
        // start selection!
        if (!hasSelection()) {
            cursorAction = CursorAction::SELECTION_START; // move to hovertarget??
            QPoint point = mapPointToImage(event->pos() * dpr);
            selectionRect.setTopLeft(point);
            selectionRect.setBottomRight(point);
            resizeAnchor = point;
        }
    } else if (event->buttons() & Qt::RightButton) {
        clearSelection();
        cursorAction = CursorAction::NO_DRAG;
    }
}

//------------------------------------------------------------------------------
// TODO: uneven movement with fractional scaling
void CropOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton /*&& hasSelection()*/) {
        if (cursorAction == CursorAction::SELECTION_START) {
            // skip if cursor hasn't been moved in some direction
            if (event->pos().x() == moveStartPos.x() || event->pos().y() == moveStartPos.y())
                return;
            // determine direction
            if (event->pos().x() > moveStartPos.x())
                if (event->pos().y() > moveStartPos.y())
                    cursorAction = CursorAction::DRAG_BOTTOMRIGHT;
                else
                    cursorAction = CursorAction::DRAG_TOPRIGHT;
            else if (event->pos().y() > moveStartPos.y())
                cursorAction = CursorAction::DRAG_BOTTOMLEFT;
            else
                cursorAction = CursorAction::DRAG_TOPLEFT;
        }
        if (cursorAction == CursorAction::NO_DRAG || cursorAction == CursorAction::SELECTION_START)
            return;
        // continue with resizing
        QPointF delta = QPointF(event->pos() - moveStartPos) * dpr;
        if (cursorAction == CursorAction::DRAG_MOVE) { // moving selection
            setCursor(Qt::ClosedHandCursor);
            moveStartPos = event->pos();
            selectionRect.translate(delta.toPoint() / scale);
            if (!imageRect.contains(selectionRect))
                selectionRect = placeInside(selectionRect, imageRect);
            updateSelectionDrawRect();
            updateHandlePositions();
            update();
#if 0
        } else if(cursorAction == CursorAction::NO_DRAG || cursorAction == CursorAction::SELECTION_START) {
            // unneeded?
            endPos = event->pos();
            //build selection rectangle
            QPoint tl, br;
            startPos.x() >= endPos.x() ? tl.setX(endPos.x()) : tl.setX(startPos.x());
            startPos.y() >= endPos.y() ? tl.setY(endPos.y()) : tl.setY(startPos.y());

            startPos.x() <= endPos.x() ? br.setX(endPos.x()) : br.setX(startPos.x());
            startPos.y() <= endPos.y() ? br.setY(endPos.y()) : br.setY(startPos.y());

            selectionRect.setTopLeft(mapPointToImage(tl * dpr));
            selectionRect.setBottomRight(mapPointToImage(br * dpr));

            updateSelectionDrawRect();
            updateHandlePositions();
            update();
#endif
        } else { // resizing selection
            resizeSelection(delta.toPoint() / scale);
            moveStartPos = event->pos();
            update();
        }
        emit selectionChanged(selectionRect);
    } else {
        if (!hasSelection())
            setCursor(QCursor(Qt::ArrowCursor));
        else
            setCursorAction(hoverTarget(event->pos() * dpr));
    }
}

//------------------------------------------------------------------------------
void CropOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    // user just clicked without moving the mouse, clear
    if (cursorAction == CursorAction::SELECTION_START)
        clearSelection();
    cursorAction = CursorAction::NO_DRAG;
    setCursorAction(hoverTarget(event->pos() * dpr));
    update();
}

//------------------------------------------------------------------------------
void CropOverlay::updateHandlePositions()
{
    // top left
    handles[0].moveTopLeft(selectionDrawRect.topLeft());
    // top right
    handles[1].moveTopRight(selectionDrawRect.topRight() + QPoint(1, 0));
    // bottom left
    handles[2].moveBottomLeft(selectionDrawRect.bottomLeft() + QPoint(0, 1));
    // bottom right
    handles[3].moveBottomRight(selectionDrawRect.bottomRight() + QPoint(1, 1));
    // left
    handles[4].moveTopLeft(QPoint(selectionDrawRect.left(), selectionDrawRect.center().y() - handleSize));
    // right
    handles[5].moveTopRight(QPoint(selectionDrawRect.right() + 1, selectionDrawRect.center().y() - handleSize));
    // top
    handles[6].moveTopLeft(QPoint(selectionDrawRect.center().x() - handleSize, selectionDrawRect.top()));
    // bottom
    handles[7].moveBottomLeft(QPoint(selectionDrawRect.center().x() - handleSize, selectionDrawRect.bottom() + 1));
    for (int i = 0; i < 8; i++)
        handlesDpi[i] = QRectF(handles[i].topLeft() / dpr, handles[i].size() / dpr);
    // top left
    handlesDpi[0].moveTopLeft(selectionDrawRectDpi.topLeft());
    // top right
    handlesDpi[1].moveTopRight(selectionDrawRectDpi.topRight() + QPoint(1, 0));
    // bottom left
    handlesDpi[2].moveBottomLeft(selectionDrawRectDpi.bottomLeft() + QPoint(0, 1));
    // bottom right
    handlesDpi[3].moveBottomRight(selectionDrawRectDpi.bottomRight() + QPoint(1, 1));
    // left
    handlesDpi[4].moveLeft(selectionDrawRectDpi.left());
    // right
    handlesDpi[5].moveRight(selectionDrawRectDpi.right() + 1);
    // top
    handlesDpi[6].moveTop(selectionDrawRectDpi.top());
    // bottom
    handlesDpi[7].moveBottom(selectionDrawRectDpi.bottom() + 1);
}

//------------------------------------------------------------------------------
// TODO: use aspect ratio
void CropOverlay::onSelectionOutsideChange(QRect selection)
{
    if (selection.width() > 0 && selection.height() > 0) {
        QRect boundedSelection = placeInside(selection, imageRect);
        selectionRect          = boundedSelection;
        updateSelectionDrawRect();
        updateHandlePositions();
        update();
        if (selectionRect != selection)
            emit selectionChanged(selectionRect);
    } else {
        // selectAll();
    }
}

//------------------------------------------------------------------------------
void CropOverlay::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && hasSelection()) {
        if (event->modifiers() == Qt::ShiftModifier)
            emit cropSave();
        else
            emit cropDefault();
    } else if (event->key() == Qt::Key_Escape) {
        clearSelection();
        emit escPressed();
    } else if (event->matches(QKeySequence::SelectAll)) {
        selectAll();
    } else {
        event->ignore();
    }
}

//------------------------------------------------------------------------------
void CropOverlay::resizeEvent(QResizeEvent *event)
{
    updateSelectionDrawRect();
    updateHandlePositions();
    update();
    QWidget::resizeEvent(event);
}

//------------------------------------------------------------------------------
void CropOverlay::recalculateGeometry()
{
    setGeometry(0, 0, containerSize().width(), containerSize().height());
}
