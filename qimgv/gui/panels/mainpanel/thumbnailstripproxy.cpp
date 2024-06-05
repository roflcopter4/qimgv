#include "thumbnailstripproxy.h"

ThumbnailStripProxy::ThumbnailStripProxy(QWidget *parent)
    : QWidget(parent),
      layout(new QVBoxLayout(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
}

ThumbnailStripProxy::~ThumbnailStripProxy()
{
}

void ThumbnailStripProxy::init()
{
    qApp->processEvents(); // chew through events in case we have something that alters stateBuf in queue
    QMutexLocker ml(&m);
    if (thumbnailStrip)
        return;
    thumbnailStrip = new ThumbnailStrip(this);
    ml.unlock();
    layout->addWidget(thumbnailStrip);
    setFocusProxy(thumbnailStrip);
    setLayout(layout);

    connect(thumbnailStrip, &ThumbnailStrip::itemActivated,       this, &ThumbnailStripProxy::itemActivated);
    connect(thumbnailStrip, &ThumbnailStrip::thumbnailsRequested, this, &ThumbnailStripProxy::thumbnailsRequested);

    thumbnailStrip->show();

    // apply buffer
    thumbnailStrip->populate(stateBuf.itemCount);
    thumbnailStrip->select(stateBuf.selection);
    // wait till layout stuff happens
    // before calling focusOn()
    qApp->processEvents();
    thumbnailStrip->focusOnSelection();
}

bool ThumbnailStripProxy::isInitialized() const
{
    return thumbnailStrip != nullptr;
}

void ThumbnailStripProxy::populate(qsizetype count)
{
    QMutexLocker ml(&m);
    stateBuf.itemCount = count;
    if (thumbnailStrip) {
        ml.unlock();
        thumbnailStrip->populate(stateBuf.itemCount);
    } else {
        stateBuf.selection.clear();
    }
}

void ThumbnailStripProxy::setThumbnail(qsizetype pos, QSharedPointer<Thumbnail> thumb)
{
    if (thumbnailStrip)
        thumbnailStrip->setThumbnail(pos, thumb);
}

void ThumbnailStripProxy::select(SelectionList indices)
{
    if (thumbnailStrip)
        thumbnailStrip->select(indices);
    else
        stateBuf.selection = indices;
}

void ThumbnailStripProxy::select(qsizetype index)
{
    if (thumbnailStrip) {
        thumbnailStrip->select(index);
    } else {
        stateBuf.selection.clear();
        stateBuf.selection << index;
    }
}

IDirectoryView::SelectionList &ThumbnailStripProxy::selection()
{
    return thumbnailStrip ? thumbnailStrip->selection() : stateBuf.selection;
}

IDirectoryView::SelectionList const &ThumbnailStripProxy::selection() const
{
    return thumbnailStrip ? thumbnailStrip->selection() : stateBuf.selection;
}

void ThumbnailStripProxy::focusOn(qsizetype index)
{
    if (thumbnailStrip)
        thumbnailStrip->focusOn(index);
}

void ThumbnailStripProxy::focusOnSelection()
{
    if (thumbnailStrip)
        thumbnailStrip->focusOnSelection();
}

void ThumbnailStripProxy::insertItem(qsizetype index)
{
    if (thumbnailStrip)
        thumbnailStrip->insertItem(index);
    else
        stateBuf.itemCount++;
}

void ThumbnailStripProxy::removeItem(qsizetype index)
{
    if (thumbnailStrip) {
        thumbnailStrip->removeItem(index);
    } else {
        --stateBuf.itemCount;
        stateBuf.selection.removeAll(index);
        for (qsizetype i = 0; i < stateBuf.selection.count(); i++)
            if (stateBuf.selection[i] > index)
                --stateBuf.selection[i];
        if (!stateBuf.selection.count())
            stateBuf.selection << ((index >= stateBuf.itemCount) ? stateBuf.itemCount - 1 : index);
    }
}

void ThumbnailStripProxy::reloadItem(qsizetype index)
{
    if (thumbnailStrip)
        thumbnailStrip->reloadItem(index);
}

void ThumbnailStripProxy::setDragHover(qsizetype index)
{
    if (thumbnailStrip)
        thumbnailStrip->setDragHover(index);
}

void ThumbnailStripProxy::setDirectoryPath(QString path)
{
    Q_UNUSED(path)
}

void ThumbnailStripProxy::addItem()
{
    if (thumbnailStrip)
        thumbnailStrip->addItem();
    else
        stateBuf.itemCount++;
}

QSize ThumbnailStripProxy::itemSize() const
{
    return thumbnailStrip->itemSize();
}

void ThumbnailStripProxy::readSettings()
{
    if (thumbnailStrip)
        thumbnailStrip->readSettings();
}

void ThumbnailStripProxy::showEvent(QShowEvent *event)
{
    init();
    QWidget::showEvent(event);
}
