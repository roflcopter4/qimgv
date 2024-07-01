#include "ThumbnailStrip.h"

// TODO: move item highlight logic to base class

ThumbnailStrip::ThumbnailStrip(QWidget *parent)
    : ThumbnailView(Qt::Horizontal, parent)
{
    setAttribute(Qt::WA_NoMousePropagation);
    setFocusPolicy(Qt::NoFocus);
    setupLayout();
    readSettings();
}

void ThumbnailStrip::updateScrollbarIndicator()
{
    if (!thumbnails.count() || lastSelected() == -1)
        return;

    qreal itemCenter = (qreal(lastSelected()) + 0.5) / qreal(itemCount());

    if (scrollBar->orientation() == Qt::Horizontal)
        indicator = QRect(int(scrollBar->width() * itemCenter - indicatorSize), 2,
                          int(indicatorSize), scrollBar->height() - 4);
    else
        indicator = QRect(2, int(scrollBar->height() * itemCenter - indicatorSize),
                          scrollBar->width() - 4, int(indicatorSize));
}

// No layout; Manual item positioning.
// Graphical issues otherwise.
void ThumbnailStrip::setupLayout()
{
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

ThumbnailWidget *ThumbnailStrip::createThumbnailWidget()
{
    auto *widget = new ThumbnailWidget();
    widget->setPadding(thumbPadding);
    widget->setMargins(thumbMarginX, thumbMarginY);
    widget->setThumbStyle(mCurrentStyle);
    widget->setThumbnailSize(mThumbnailSize);
    return widget;
}

void ThumbnailStrip::addItemToLayout(ThumbnailWidget *widget, qsizetype pos)
{
    scene->addItem(widget);
    updateThumbnailPositions(pos, thumbnails.count() - 1);
}

void ThumbnailStrip::removeItemFromLayout(qsizetype pos)
{
    if (checkRange(pos)) {
        ThumbnailWidget *thumb = thumbnails[pos];
        scene->removeItem(thumb);
        // move items
        if (orientation() == Qt::Horizontal) {
            for (auto *th : thumbnails)
                th->moveBy(-th->boundingRect().width(), 0);
        } else {
            for (auto *th : thumbnails)
                th->moveBy(0, -th->boundingRect().height());
        }
    }
}

void ThumbnailStrip::removeAll()
{
    scene->clear(); // also calls delete on all items
    thumbnails.clear();
}

void ThumbnailStrip::updateThumbnailPositions()
{
    updateThumbnailPositions(0, thumbnails.count() - 1);
}

void ThumbnailStrip::updateThumbnailPositions(qsizetype start, qsizetype end)
{
    if (start > end || !checkRange(start) || !checkRange(end))
        return;
    // assume all thumbnails are the same size
    if (orientation() == Qt::Horizontal) {
        auto thumbWidth = thumbnails[start]->boundingRect().width();
        for (qsizetype i = start; i <= end; ++i)
            thumbnails[i]->setPos(static_cast<qreal>(i) * thumbWidth, 0);
    } else {
        auto thumbHeight = thumbnails[start]->boundingRect().height();
        for (qsizetype i = start; i <= end; ++i)
            thumbnails[i]->setPos(0, static_cast<qreal>(i) * thumbHeight);
    }
}

void ThumbnailStrip::focusOn(qsizetype index)
{
    if (!checkRange(index))
        return;
    auto const *th = thumbnails[index];
    if (settings->panelCenterSelection()) {
        QGraphicsView::centerOn(th->sceneBoundingRect().center());
    } else {
        // partially show the next thumb if possible
        if (orientation() == Qt::Vertical) {
            if (height() > th->height() * 2)
                ensureVisible(th, 0, static_cast<int>(th->height() / 2));
            else
                ensureVisible(th, 0, 0);
        } else if (width() > th->width() * 2.0) {
            ensureVisible(th, static_cast<int>(th->width() / 2.0), 0);
        } else {
            ensureVisible(th, 0, 0);
        }
    }
    loadVisibleThumbnails();
}

void ThumbnailStrip::focusOnSelection()
{
    if (selection().isEmpty())
        return;
    focusOn(selection().last());
}

void ThumbnailStrip::readSettings()
{
    if (settings->thumbPanelStyle() == ThumbPanelStyle::SIMPLE)
        mCurrentStyle = ThumbnailStyle::SIMPLE;
    else
        mCurrentStyle = ThumbnailStyle::NORMAL_CENTERED;

    mThumbnailSize = qBound(20, settings->panelPreviewsSize(), 300);

    if (settings->panelPosition() == PanelPosition::TOP ||
        settings->panelPosition() == PanelPosition::BOTTOM)
    {
        setOrientation(Qt::Horizontal);
        thumbMarginX = 2;
        thumbMarginY = 4;
    } else {
        setOrientation(Qt::Vertical);
        thumbMarginX = 12;
        thumbMarginY = 2;
    }

    // apply style, size & reposition
    for (auto *th : thumbnails) {
        th->setPadding(thumbPadding);
        th->setMargins(thumbMarginX, thumbMarginY);
        th->setThumbStyle(mCurrentStyle);
        th->setThumbnailSize(mThumbnailSize);
    }
    updateThumbnailPositions(0, thumbnails.count() - 1);
    fitSceneToContents();
    setCropThumbnails(settings->squareThumbnails());
    focusOn(lastSelected());
}

QSize ThumbnailStrip::itemSize()
{
    if (thumbnails.isEmpty()) {
        ThumbnailWidget w;
        w.setPadding(thumbPadding);
        w.setMargins(thumbMarginX, thumbMarginY);
        w.setThumbStyle(mCurrentStyle);
        w.setThumbnailSize(mThumbnailSize);
        return w.boundingRect().size().toSize();
    }
    return thumbnails[0]->boundingRect().size().toSize();
}

void ThumbnailStrip::resizeEvent(QResizeEvent *event)
{
    ThumbnailView::resizeEvent(event);
    fitSceneToContents();
    if (event->oldSize().width() < width())
        loadVisibleThumbnailsDelayed();
}
