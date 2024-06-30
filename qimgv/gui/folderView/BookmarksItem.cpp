#include "BookmarksItem.h"

BookmarksItem::BookmarksItem(QString const &dirName, QString const &dirPath, QWidget *parent)
    : QWidget(parent),
      dirName(dirName),
      dirPath(dirPath),
      layout(new QHBoxLayout(this)),
      mHighlighted(false)
{
    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(10, 6, 10, 6);
    setAcceptDrops(true);
    dirNameLabel.setText(dirName);

    spacer = new QSpacerItem(16, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    folderIconWidget.setAttribute(Qt::WA_TransparentForMouseEvents, true);
    folderIconWidget.setIconPath(u":/res/icons/common/menuitem/folder16.png"_s);
    folderIconWidget.setMinimumSize(16, 16);
    folderIconWidget.installEventFilter(this);

    removeItemButton.setIconPath(u":/res/icons/common/buttons/panel-small/remove12.png"_s);
    removeItemButton.setMinimumSize(16, 16);
    removeItemButton.installEventFilter(this);
    removeItemButton.setAccessibleName(u"BookmarksItemRemoveLabel"_s);

    connect(&removeItemButton, &IconButton::clicked, this, &BookmarksItem::onRemoveClicked);

    layout->addWidget(&folderIconWidget);
    layout->addWidget(&dirNameLabel);
    layout->addSpacerItem(spacer);
    layout->addWidget(&removeItemButton);

    setMouseTracking(true);
    setLayout(layout);
}

QString BookmarksItem::path() const
{
    return dirPath;
}

void BookmarksItem::setHighlighted(bool mode)
{
    if (mode != mHighlighted) {
        mHighlighted = mode;
        setProperty("highlighted", mHighlighted);
        style()->unpolish(this);
        style()->polish(this);
    }
}

void BookmarksItem::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    if (event->button() == Qt::LeftButton)
        emit clicked(dirPath);
}

void BookmarksItem::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void BookmarksItem::onRemoveClicked()
{
    emit removeClicked(dirPath);
}

void BookmarksItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void BookmarksItem::dropEvent(QDropEvent *event)
{
    QList<QString> paths;
    // TODO: QUrl gave me some issues previosly, test
    for (auto const &url : event->mimeData()->urls())
        paths << url.toLocalFile();
    emit droppedIn(paths, dirPath);
}

void BookmarksItem::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
    setProperty("hover", true);
    update();
}

void BookmarksItem::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    setProperty("hover", false);
    update();
}
