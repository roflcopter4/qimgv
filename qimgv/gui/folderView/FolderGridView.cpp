#include "FolderGridView.h"

// TODO: create a base class for this and the one on panel

FolderGridView::FolderGridView(QWidget *parent)
    : ThumbnailView(Qt::Vertical, parent),
      holderWidget(new QGraphicsWidget()),
      shiftedCol(-1)
{
    offscreenPreloadArea = 2300;

    setAcceptDrops(true);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    scene->setBackgroundBrush(settings->colorScheme().folderview);
    setCacheMode(QGraphicsView::CacheBackground);

    // turn this off until [multi]selection is implemented
    setDrawScrollbarIndicator(false);
    setSelectMode(ThumbnailSelectMode::ACTIVATE_BY_DOUBLECLICK);

    connect(settings, &Settings::settingsChanged, this, &FolderGridView::onSettingsChanged);
    setupLayout();
    connect(this, &ThumbnailView::itemActivated, this, &FolderGridView::onitemSelected);
}

FolderGridView::~FolderGridView() = default;

void FolderGridView::dropEvent(QDropEvent *event)
{
    event->accept();
    auto     *item  = dynamic_cast<ThumbnailWidget *>(itemAt(event->position().toPoint()));
    qsizetype index = -1;
    if (item) {
        index = thumbnails.indexOf(item);
        item->setDropHovered(false);
    }
    emit droppedInto(event->mimeData(), event->source(), index);
}

void FolderGridView::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void FolderGridView::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
    auto     *item  = dynamic_cast<ThumbnailWidget *>(itemAt(event->position().toPoint()));
    qsizetype index = -1;
    if (item)
        index = thumbnails.indexOf(item);
    // unselect previous
    if (index != lastDragTarget && checkRange(lastDragTarget))
        thumbnails[lastDragTarget]->setDropHovered(false);
    emit draggedOver(index);
    lastDragTarget = index;
}

void FolderGridView::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
    if (lastDragTarget < 0 || lastDragTarget >= thumbnails.count())
        return;
    thumbnails[lastDragTarget]->setDropHovered(false);
}

void FolderGridView::setDragHover(qsizetype index)
{
    if (!checkRange(index))
        return;
    auto item = thumbnails[index];
    item->setDropHovered(true);
}

void FolderGridView::onitemSelected()
{
    shiftedCol = -1;
}

void FolderGridView::onSettingsChanged()
{
    scene->setBackgroundBrush(settings->colorScheme().folderview);
}

void FolderGridView::updateScrollbarIndicator()
{
    if (!thumbnails.count() || !selection().count())
        return;
    ThumbnailWidget *thumb = thumbnails[lastSelected()];
    qreal itemCenter = (thumb->pos().y() + (thumb->height() / 2.0)) / scene->height();
    indicator = QRectF(2.0, scrollBar->height() * itemCenter - indicatorSize, scrollBar->width() - 4, indicatorSize).toRect();
}

// probably unneeded
void FolderGridView::show()
{
    ThumbnailView::show();
    setFocus();
}

// probably unneeded
void FolderGridView::hide()
{
    ThumbnailView::hide();
    clearFocus();
}

void FolderGridView::setShowLabels(bool mode)
{
    auto style = mode ? ThumbnailStyle::NORMAL : ThumbnailStyle::SIMPLE;
    for (qsizetype i = 0; i < thumbnails.count(); i++)
        thumbnails[i]->setThumbStyle(style);
    updateLayout();
    fitSceneToContents();
    focusOnSelection();
}

void FolderGridView::focusOnSelection()
{
    if (!thumbnails.count() || lastSelected() == -1)
        return;
    ThumbnailWidget *thumb = thumbnails.at(lastSelected());
    ensureVisible(thumb, 0, 0);
}

void FolderGridView::selectAll()
{
    SelectionList list;
    for (qsizetype i = 0; i < thumbnails.count(); i++)
        list << i;
    // preserve last selected index by putting it at the end of a new selection
    // this is simpler but it changes selection order a bit
    if (lastSelected() != -1) {
        // in this case list is sorted so no need to indexOf()
        list.move(lastSelected(), list.count() - 1);
    }
    select(list);
}

void FolderGridView::selectAbove()
{
    if (!thumbnails.count() || lastSelected() == -1 || flowLayout->sameRow(0, lastSelected()))
        return;
    qsizetype newIndex = flowLayout->itemAbove(lastSelected());
    if (shiftedCol >= 0) {
        qsizetype diff = shiftedCol - flowLayout->columnOf(lastSelected());
        newIndex += diff;
        shiftedCol = -1;
    }
    if (!checkRange(newIndex))
        newIndex = 0;
    if (rangeSelection)
        addSelectionRange(newIndex);
    else
        select(newIndex);
    scrollToCurrent();
}

void FolderGridView::selectBelow()
{
    if (!thumbnails.count() || lastSelected() == -1 || flowLayout->sameRow(lastSelected(), thumbnails.count() - 1))
        return;
    shiftedCol = -1;
    qsizetype newIndex = flowLayout->itemBelow(lastSelected());
    if (!checkRange(newIndex))
        newIndex = thumbnails.count() - 1;
    if (flowLayout->columnOf(newIndex) != flowLayout->columnOf(lastSelected()))
        shiftedCol = flowLayout->columnOf(lastSelected());
    if (rangeSelection)
        addSelectionRange(newIndex);
    else
        select(newIndex);
    scrollToCurrent();
}

void FolderGridView::selectNext()
{
    if (!thumbnails.count() || lastSelected() == thumbnails.count() - 1)
        return;
    if (!rangeSelection && lastSelected() == thumbnails.count() - 1) {
        select(lastSelected());
        return;
    }
    shiftedCol = -1;
    qsizetype newIndex = lastSelected() + 1;
    if (!checkRange(newIndex))
        newIndex = thumbnails.count() - 1;
    if (rangeSelection)
        addSelectionRange(newIndex);
    else
        select(newIndex);
    scrollToCurrent();
}

void FolderGridView::selectPrev()
{
    if (!thumbnails.count() || lastSelected() == 0)
        return;
    shiftedCol = -1;
    qsizetype newIndex = lastSelected() - 1;
    if (!checkRange(newIndex))
        newIndex = 0;
    if (rangeSelection)
        addSelectionRange(newIndex);
    else
        select(newIndex);
    scrollToCurrent();
}

void FolderGridView::pageUp()
{
    if (!thumbnails.count() || lastSelected() == -1 || flowLayout->sameRow(0, lastSelected()))
        return;
    qsizetype newIndex = lastSelected();
    // 4 rows up
    for (int i = 0; i < 4; ++i) {
        qsizetype tmp = flowLayout->itemAbove(newIndex);
        if (checkRange(tmp))
            newIndex = tmp;
    }
    if (shiftedCol >= 0) {
        qsizetype diff = shiftedCol - flowLayout->columnOf(newIndex);
        newIndex += diff;
        shiftedCol = -1;
    }
    if (rangeSelection)
        addSelectionRange(newIndex);
    else
        select(newIndex);
    scrollToCurrent();
}

void FolderGridView::pageDown()
{
    if (!thumbnails.count() || lastSelected() == -1 || flowLayout->sameRow(lastSelected(), thumbnails.count() - 1))
        return;
    shiftedCol   = -1;
    qsizetype newIndex = lastSelected();
    // 4 rows down
    for (int i = 0; i < 4; i++) {
        qsizetype tmp = flowLayout->itemBelow(newIndex);
        if (checkRange(tmp))
            newIndex = tmp;
    }
    if (flowLayout->columnOf(newIndex) != flowLayout->columnOf(lastSelected()))
        shiftedCol = flowLayout->columnOf(lastSelected());
    if (rangeSelection)
        addSelectionRange(newIndex);
    else
        select(newIndex);
    scrollToCurrent();
}

void FolderGridView::selectFirst()
{
    if (!thumbnails.count())
        return;
    shiftedCol = -1;
    if (rangeSelection)
        addSelectionRange(0);
    else
        select(0);
    scrollToCurrent();
}

void FolderGridView::selectLast()
{
    if (!thumbnails.count())
        return;
    shiftedCol = -1;
    if (rangeSelection)
        addSelectionRange(thumbnails.count() - 1);
    else
        select(thumbnails.count() - 1);
    scrollToCurrent();
}

void FolderGridView::scrollToCurrent()
{
    scrollToItem(lastSelected());
}

// same as scrollToItem minus the animation
void FolderGridView::focusOn(qsizetype index)
{
    if (!checkRange(index))
        return;
    ThumbnailWidget *thumb = thumbnails[index];
    ensureVisible(thumb, 0, 0);
    loadVisibleThumbnailsDelayed();
}

void FolderGridView::setupLayout()
{
    setAlignment(Qt::AlignHCenter);

    flowLayout = new FlowLayout();
    flowLayout->setContentsMargins(9, 6, 9, 0);
    setFrameShape(QFrame::NoFrame);
    scene->addItem(holderWidget);
    holderWidget->setLayout(flowLayout);
    holderWidget->setContentsMargins(0, 0, 0, 0);
}

ThumbnailWidget *FolderGridView::createThumbnailWidget()
{
    auto *widget = new ThumbnailWidget();
    widget->setPadding(8);

    ThumbnailStyle style = settings->folderViewMode() == FolderViewMode::SIMPLE
                            ? ThumbnailStyle::SIMPLE
                            : ThumbnailStyle::NORMAL;

    widget->setThumbStyle(style);
    widget->setThumbnailSize(mThumbnailSize); // TODO: constructor
    return widget;
}

void FolderGridView::addItemToLayout(ThumbnailWidget *widget, qsizetype pos)
{
    scene->addItem(widget);
    flowLayout->insertItem(pos, widget);
}

void FolderGridView::removeItemFromLayout(qsizetype pos)
{
    flowLayout->removeAt(static_cast<int>(pos));
}

void FolderGridView::removeAll()
{
    flowLayout->clear();
    qDeleteAll(thumbnails);
    thumbnails.clear();
}

void FolderGridView::updateLayout()
{
    shiftedCol = -1;
    flowLayout->invalidate();
    flowLayout->activate();
}

// block native tab-switching so we can use it in shortcuts
bool FolderGridView::focusNextPrevChild(bool)
{
    return false;
}

void FolderGridView::keyPressEvent(QKeyEvent *event)
{
    ThumbnailView::keyPressEvent(event);
    event->accept();

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit itemActivated(lastSelected());
        return;
    }

    // temporary, will be configurable later
    if (event->key() == Qt::Key_Backspace) {
        actionManager->invokeAction(u"goUp"_s);
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        if (ShortcutBuilder::fromEvent(event) == u"Ctrl+A"_sv)
            selectAll();
        else
            actionManager->processEvent(event);
        return;
    }

    // handle selection
    switch (event->key()) {
    case Qt::Key_Left:     selectPrev();  break;
    case Qt::Key_Right:    selectNext();  break;
    case Qt::Key_Up:       selectAbove(); break;
    case Qt::Key_Down:     selectBelow(); break;
    case Qt::Key_PageUp:   pageUp();      break;
    case Qt::Key_PageDown: pageDown();    break;
    case Qt::Key_Home:     selectFirst(); break;
    case Qt::Key_End:      selectLast();  break;
    default:
        actionManager->processEvent(event);
    }
}

void FolderGridView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        if (event->pixelDelta().y() > 0 || event->angleDelta().y() > 0)
            zoomIn();
        else if (event->pixelDelta().y() < 0 || event->angleDelta().y() < 0)
            zoomOut();
    } else {
        ThumbnailView::wheelEvent(event);
    }
}

void FolderGridView::zoomIn()
{
    setThumbnailSize(mThumbnailSize + ZOOM_STEP);
}

void FolderGridView::zoomOut()
{
    setThumbnailSize(mThumbnailSize - ZOOM_STEP);
}

void FolderGridView::setThumbnailSize(int newSize)
{
    newSize        = std::clamp(newSize, THUMBNAIL_SIZE_MIN, THUMBNAIL_SIZE_MAX);
    mThumbnailSize = newSize;
    for (qsizetype i = 0; i < thumbnails.count(); i++)
        thumbnails[i]->setThumbnailSize(newSize);
    updateLayout();
    fitSceneToContents();
    if (lastSelected() != -1)
        ensureVisible(thumbnails.at(lastSelected()), 0, 40);
    emit thumbnailSizeChanged(mThumbnailSize);
    loadVisibleThumbnails();
}

void FolderGridView::fitSceneToContents()
{
    if (scrollBar->isVisible())
        holderWidget->setGeometry(0, 0, width() - scrollBar->width(), height());
    else
        holderWidget->setGeometry(0, 0, width(), height());
    ThumbnailView::fitSceneToContents();
}

void FolderGridView::resizeEvent(QResizeEvent *event)
{
    if (isVisible()) {
        ThumbnailView::resizeEvent(event);
        fitSceneToContents();
        // focusOn(selectedIndex());
        loadVisibleThumbnailsDelayed();
    }
}
