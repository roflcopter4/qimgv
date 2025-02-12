#include "TreeViewCustom.h"
#include <QFileSystemModel>
#include <QTimer>

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

#if 0
void TreeViewCustom::continueExpansion(QModelIndex const &index)
{
    expand(index);
    auto *fileSystemModel = reinterpret_cast<QFileSystemModel const *>(index.model());
    //if (!fileSystemModel)
    //    return; // Handle the case where the model isn't a QFileSystemModel

    // Iterate over the children and call expandRecursively for each directory
    for (int i = 0; i < fileSystemModel->rowCount(index); ++i) {
        QModelIndex childIndex = fileSystemModel->index(i, index.column(), index);
        if (fileSystemModel->isDir(childIndex))
            reallyExpandRecursively(childIndex);
    }
}

void TreeViewCustom::reallyExpandRecursively(QModelIndex const &index)
{
    // Use qobject_cast for safety
    auto *fileSystemModel = qobject_cast<QFileSystemModel const *>(index.model());
    if (!fileSystemModel)
        return; // Handle the case where the model isn't a QFileSystemModel

    if (!index.isValid() || !fileSystemModel->isDir(index))
        return;

    // Expand the current index.
    expand(index);

    // Use a single-shot timer to avoid busy waiting and potential stack overflow.
    QTimer::singleShot(
        0, this, 
        [this, index, fileSystemModel]
    {
            // Check if there are any directories which are not loaded yet.
            bool hasChildren = fileSystemModel->hasChildren(index);
            bool canFetchMore = fileSystemModel->canFetchMore(index);

            // Connect to directoryLoaded to be notified when the directory has been fully loaded
            //auto *fileSystemModel = reinterpret_cast<QFileSystemModel const *>(index.model());
            if (hasChildren) {
                connect(
                    fileSystemModel, &QFileSystemModel::directoryLoaded, this,
                    [this, index, fileSystemModel](QString const &path) {
                        qDebug() << path;
                        auto newPath = fileSystemModel->filePath(index);
                        continueExpansion(index);
                        if (newPath == path) {
                            disconnect(fileSystemModel, &QFileSystemModel::directoryLoaded, this, nullptr);
                        }
                    }
                );
            }
            // If the directory is already loaded then continue expansion.
            else {
                continueExpansion(index);
                disconnect(fileSystemModel, &QFileSystemModel::directoryLoaded, this, nullptr);
            }
        }
    );
}
#endif

void TreeViewCustom::reallyExpandRecursively(QModelIndex const &index)
{
    if (!index.isValid())
        return;
    auto *model = reinterpret_cast<QFileSystemModel const *>(index.model());
    auto  dir   = QDir(model->filePath(index));
    auto  list  = dir.entryList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot);

    expand(index);
    int iterations = 0;
    int cnt;
    do {
        QCoreApplication::processEvents();
        QThread::sleep(5ms);
        cnt = model->rowCount(index);
    } while (cnt < list.size() && ++iterations < 1024);

    for (int i = 0; i < cnt; ++i)
        reallyExpandRecursively(model->index(i, index.column(), index));
}

void TreeViewCustom::keyPressEvent(QKeyEvent *event)
{
    QModelIndex index = currentIndex();

    if (index.isValid()) {
        switch (event->key()) {
        case Qt::Key_Space: {
            if (!isExpanding.test_and_set()) {
                QTimer::singleShot(0, this, [this, index] {
                    reallyExpandRecursively(index);
                    expandRecursively(index);
                    qDebug() << u"Finished, at last!";
                    isExpanding.clear();
                });
            }
            break;
        }
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

