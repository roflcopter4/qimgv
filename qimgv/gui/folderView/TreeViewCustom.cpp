#include "TreeViewCustom.h"

TreeViewCustom::TreeViewCustom(QWidget *parent)
    : QTreeView(parent)
{
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragEnabled(true);

    // proxy scrollbar
    verticalScrollBar()->setStyleSheet(u"max-width: 0px;"_s);
    overlayScrollbar.setParent(this);
    overlayScrollbar.setStyleSheet(u"background-color: transparent;"_s);

    connect(verticalScrollBar(), &QScrollBar::rangeChanged, &overlayScrollbar, &QScrollBar::setRange);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, &overlayScrollbar, &QScrollBar::setValue);
    connect(&overlayScrollbar,   &QScrollBar::valueChanged, this,              &TreeViewCustom::onValueChanged);
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
    return {QTreeView::minimumSizeHint().width(), 0};
}

void TreeViewCustom::resizeEvent(QResizeEvent *event)
{
    QTreeView::resizeEvent(event);
    updateScrollbarStyle();
}

void TreeViewCustom::updateScrollbarStyle()
{
    QString hover  = settings->colorScheme().scrollbar_hover.name();
    QString handle = rect().contains(mapFromGlobal(QCursor::pos())) ? settings->colorScheme().scrollbar.name()
                                                                    : settings->colorScheme().folderview_hc.name();

    overlayScrollbar.setGeometry(width() - SCROLLBAR_WIDTH, 0, SCROLLBAR_WIDTH, height());
    overlayScrollbar.setStyleSheet(
        u"QScrollBar { background-color: transparent; } "
        u"QScrollBar::handle:vertical { background-color: " + handle + u" } "
        u"QScrollBar::handle:vertical:hover { background-color: " + hover + u" }");

    overlayScrollbar.setVisible(verticalScrollBar()->maximum());
}

void TreeViewCustom::onValueChanged()
{
    verticalScrollBar()->setValue(overlayScrollbar.value());
}

void TreeViewCustom::keyPressEvent(QKeyEvent *event)
{
    QModelIndex index = currentIndex();
    if (index.isValid()) {
        switch (event->key()) {
        case Qt::Key_Space:
            expandRecursively(index);
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit clicked(index);
            break;
        default:
            QTreeView::keyPressEvent(event);
            break;
        }
    } else {
        QTreeView::keyPressEvent(event);
    }
}
