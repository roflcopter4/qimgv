#include "treeviewcustom.h"

TreeViewCustom::TreeViewCustom(QWidget *parent) : QTreeView(parent)
{
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragEnabled(true);

    // proxy scrollbar
    this->verticalScrollBar()->setStyleSheet(QS("max-width: 0px;"));
    overlayScrollbar.setParent(this);
    overlayScrollbar.setStyleSheet(QS("background-color: transparent;"));

    connect(verticalScrollBar(), &QScrollBar::rangeChanged, &overlayScrollbar, &QScrollBar::setRange);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, &overlayScrollbar, &QScrollBar::setValue);
    connect(&overlayScrollbar, &QScrollBar::valueChanged,
            [this] { verticalScrollBar()->setValue(overlayScrollbar.value()); });
}

void TreeViewCustom::dropEvent(QDropEvent *event)
{
    QModelIndex dropIndex = indexAt(event->position().toPoint());
    if (dropIndex.isValid()) {
        QList<QString> paths;
        // TODO: QUrl gave me some issues previosly, test
        for (auto const &url : event->mimeData()->urls())
            paths << url.toLocalFile();
        emit droppedIn(paths, dropIndex);
    }
}

void TreeViewCustom::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void TreeViewCustom::showEvent(QShowEvent *event)
{
    QTreeView::showEvent(event);
}

void TreeViewCustom::enterEvent(QEnterEvent *event)
{
    QTreeView::enterEvent(event);
    updateScrollbarStyle();
}
void TreeViewCustom::leaveEvent(QEvent *event)
{
    QTreeView::leaveEvent(event);
    updateScrollbarStyle();
}

QSize TreeViewCustom::minimumSizeHint() const
{
    QSize sz(QTreeView::minimumSizeHint().width(), 0);
    return sz;
}

void TreeViewCustom::resizeEvent(QResizeEvent *event)
{
    QTreeView::resizeEvent(event);
    updateScrollbarStyle();
}

void TreeViewCustom::updateScrollbarStyle()
{
    QString handle, hover = settings->colorScheme().scrollbar_hover.name();
    if (rect().contains(mapFromGlobal(QCursor::pos())))
        handle = settings->colorScheme().scrollbar.name();
    else
        handle = settings->colorScheme().folderview_hc.name();
    overlayScrollbar.setGeometry(width() - SCROLLBAR_WIDTH, 0, SCROLLBAR_WIDTH, height());
    overlayScrollbar.setStyleSheet(
        QS("QScrollBar { background-color: transparent; } QScrollBar::handle:vertical { background-color: ") +
        handle + QS(" } QScrollBar::handle:vertical:hover { background-color: ") +
        hover + QS(" }"));

    overlayScrollbar.setVisible((this->verticalScrollBar()->maximum()));
}

void TreeViewCustom::keyPressEvent(QKeyEvent *event)
{
    QModelIndex currentIndex = this->currentIndex();

    if (currentIndex.isValid()) {
        switch (event->key()) {
        case Qt::Key_Space:
            emit expandRecursively(currentIndex);
            //expand(currentIndex);
            break;
            //[[fallthrough]];
        case Qt::Key_Enter:
        case Qt::Key_Return:
            emit clicked(currentIndex);
            break;
        default:
            QTreeView::keyPressEvent(event);
            break;
        }
    } else {
            QTreeView::keyPressEvent(event);
    }
#if 0
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && currentIndex.isValid())
        emit clicked(currentIndex);
    else
        QTreeView::keyPressEvent(event);
#endif
}
