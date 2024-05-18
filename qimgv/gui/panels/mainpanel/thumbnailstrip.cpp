#include "thumbnailstrip.h"

// TODO: move item highlight logic to base class

ThumbnailStrip::ThumbnailStrip(QWidget *parent)
    : ThumbnailView(Qt::Horizontal, parent)
{
    this->setAttribute(Qt::WA_NoMousePropagation, true);
    this->setFocusPolicy(Qt::NoFocus);
    setupLayout();
    readSettings();
}

void ThumbnailStrip::updateScrollbarIndicator()
{
    if (!thumbnails.count() || lastSelected() == -1)
        return;
    qreal itemCenter = (qreal)(lastSelected() + 0.5) / itemCount();
    if (scrollBar->orientation() == Qt::Horizontal)
        indicator = QRect(scrollBar->width() * itemCenter - indicatorSize, 2, indicatorSize, scrollBar->height() - 4);
    else
        indicator = QRect(2, scrollBar->height() * itemCenter - indicatorSize, scrollBar->width() - 4, indicatorSize);
}

//  no layout; manual item positioning
//  graphical issues otherwise
void ThumbnailStrip::setupLayout()
{
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
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

void ThumbnailStrip::addItemToLayout(ThumbnailWidget *widget, int pos)
{
    scene.addItem(widget);
    updateThumbnailPositions(pos, thumbnails.count() - 1);
}

void ThumbnailStrip::removeItemFromLayout(int pos)
{
    if (checkRange(pos)) {
        ThumbnailWidget *thumb = thumbnails.at(pos);
        scene.removeItem(thumb);
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
    scene.clear(); // also calls delete on all items
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
        int thumbWidth = static_cast<int>(thumbnails.at(start)->boundingRect().width());
        for (qsizetype i = start; i <= end; i++)
            thumbnails[i]->setPos(i * thumbWidth, 0);
    } else {
        int thumbHeight = static_cast<int>(thumbnails.at(start)->boundingRect().height());
        for (qsizetype i = start; i <= end; i++)
            thumbnails[i]->setPos(0, i * thumbHeight);
    }
}

void ThumbnailStrip::focusOn(int index)
{
    if (!checkRange(index))
        return;
    auto const *th = thumbnails.at(index);
    if (settings->panelCenterSelection()) {
        QGraphicsView::centerOn(th->sceneBoundingRect().center());
    } else {
        // partially show the next thumb if possible
        if (orientation() == Qt::Vertical) {
            if (height() > th->height() * 2)
                ensureVisible(th, 0, th->height() / 2);
            else
                ensureVisible(th, 0, 0);
        } else if (width() > th->width() * 2) {
            ensureVisible(th, th->width() / 2, 0);
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
        ThumbnailView::setOrientation(Qt::Horizontal);
        thumbMarginX = 2;
        thumbMarginY = 4;
    } else {
        ThumbnailView::setOrientation(Qt::Vertical);
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
    } else {
        return thumbnails.at(0)->boundingRect().size().toSize();
    }
}

void ThumbnailStrip::resizeEvent(QResizeEvent *event)
{
    ThumbnailView::resizeEvent(event);
    fitSceneToContents();
    if (event->oldSize().width() < width())
        loadVisibleThumbnailsDelayed();
}
