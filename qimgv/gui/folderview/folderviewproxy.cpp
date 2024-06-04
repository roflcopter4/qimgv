#include "folderviewproxy.h"

FolderViewProxy::FolderViewProxy(QWidget *parent)
    : QWidget(parent),
      folderView(nullptr)
{
    stateBuf.sortingMode = settings->sortingMode();
    layout.setContentsMargins(0,0,0,0);
}

void FolderViewProxy::init()
{
    qApp->processEvents(); // chew through events in case we have something that alters stateBuf in queue
    QMutexLocker ml(&m);
    if (folderView)
        return;
    folderView.reset(new FolderView());
    folderView->setParent(this);
    ml.unlock();
    layout.addWidget(folderView.get());
    this->setFocusProxy(folderView.get());
    this->setLayout(&layout);

    connect(folderView.get(), &FolderView::itemActivated,       this, &FolderViewProxy::itemActivated);
    connect(folderView.get(), &FolderView::thumbnailsRequested, this, &FolderViewProxy::thumbnailsRequested);
    connect(folderView.get(), &FolderView::sortingSelected,     this, &FolderViewProxy::sortingSelected);
    connect(folderView.get(), &FolderView::showFoldersChanged,  this, &FolderViewProxy::showFoldersChanged);
    connect(folderView.get(), &FolderView::directorySelected,   this, &FolderViewProxy::directorySelected);
    connect(folderView.get(), &FolderView::draggedOut,          this, &FolderViewProxy::draggedOut);
    connect(folderView.get(), &FolderView::copyUrlsRequested,   this, &FolderViewProxy::copyUrlsRequested);
    connect(folderView.get(), &FolderView::moveUrlsRequested,   this, &FolderViewProxy::moveUrlsRequested);
    connect(folderView.get(), &FolderView::droppedInto,         this, &FolderViewProxy::droppedInto);
    connect(folderView.get(), &FolderView::draggedOver,         this, &FolderViewProxy::draggedOver);

    folderView->show();

    // apply buffer
    if (!stateBuf.directory.isEmpty())
        folderView->setDirectoryPath(stateBuf.directory);
    folderView->onFullscreenModeChanged(stateBuf.fullscreenMode);
    folderView->populate(stateBuf.itemCount);
    folderView->select(stateBuf.selection);
    // wait till layout stuff happens
    // before calling focusOn()
    qApp->processEvents();
    folderView->focusOnSelection();
    folderView->onSortingChanged(stateBuf.sortingMode);
}

void FolderViewProxy::populate(qsizetype count)
{
    QMutexLocker ml(&m);
    stateBuf.itemCount = count;
    if (folderView) {
        ml.unlock();
        folderView->populate(stateBuf.itemCount);
    } else {
        stateBuf.selection.clear();
    }
}

void FolderViewProxy::setThumbnail(qsizetype pos, std::shared_ptr<Thumbnail> thumb)
{
    if (folderView)
        folderView->setThumbnail(pos, std::move(thumb));
}

void FolderViewProxy::select(SelectionList indices)
{
    if (folderView)
        folderView->select(indices);
    else
        stateBuf.selection = indices;
}

void FolderViewProxy::select(qsizetype index)
{
    if (folderView) {
        folderView->select(index);
    } else {
        stateBuf.selection.clear();
        stateBuf.selection << index;
    }
}

FolderViewProxy::SelectionList &FolderViewProxy::selection()
{
    return folderView ? folderView->selection()
                      : stateBuf.selection;
}

FolderViewProxy::SelectionList const &FolderViewProxy::selection() const
{
    return folderView ? folderView->selection()
                      : stateBuf.selection;
}

void FolderViewProxy::focusOn(qsizetype index)
{
    if (folderView)
        folderView->focusOn(index);
}

void FolderViewProxy::focusOnSelection()
{
    if (folderView)
        folderView->focusOnSelection();
}

void FolderViewProxy::setDirectoryPath(QString path)
{
    if (folderView)
        folderView->setDirectoryPath(path);
    else
        stateBuf.directory = path;
}

void FolderViewProxy::insertItem(qsizetype index)
{
    if (folderView)
        folderView->insertItem(index);
    else
        stateBuf.itemCount++;
}

void FolderViewProxy::removeItem(qsizetype index)
{
    if (folderView) {
        folderView->removeItem(index);
    } else {
        stateBuf.itemCount--;
        stateBuf.selection.removeAll(index);
        for (qsizetype i = 0; i < stateBuf.selection.count(); ++i)
            if (stateBuf.selection[i] > index)
                stateBuf.selection[i]--;
        if (!stateBuf.selection.count())
            stateBuf.selection << ((index >= stateBuf.itemCount) ? stateBuf.itemCount - 1 : index);
    }
}

void FolderViewProxy::reloadItem(qsizetype index)
{
    if (folderView)
        folderView->reloadItem(index);
}

void FolderViewProxy::setDragHover(qsizetype index)
{
    if (folderView)
        folderView->setDragHover(index);
}

void FolderViewProxy::addItem()
{
    if (folderView)
        folderView->addItem();
    else
        stateBuf.itemCount++;
}

void FolderViewProxy::onFullscreenModeChanged(bool mode)
{
    if (folderView)
        folderView->onFullscreenModeChanged(mode);
    else
        stateBuf.fullscreenMode = mode;
}

void FolderViewProxy::onSortingChanged(SortingMode mode)
{
    if (folderView)
        folderView->onSortingChanged(mode);
    else
        stateBuf.sortingMode = mode;
}

void FolderViewProxy::showEvent(QShowEvent *event)
{
    init();
    QWidget::showEvent(event);
}
