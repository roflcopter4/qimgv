#include "DirectoryPresenter.h"

DirectoryPresenter::DirectoryPresenter(QObject *parent)
    : QObject(parent),
      thumbnailer(new Thumbnailer(this)),
      mShowDirs(false)
{
    connect(thumbnailer, &Thumbnailer::thumbnailReady, this, &DirectoryPresenter::onThumbnailReady);
}

DirectoryPresenter::~DirectoryPresenter()
{
}

void DirectoryPresenter::unsetModel()
{
    if (!model)
        return;

    disconnect(model.get(), &DirectoryModel::fileRemoved,  this, &DirectoryPresenter::onFileRemoved);
    disconnect(model.get(), &DirectoryModel::fileAdded,    this, &DirectoryPresenter::onFileAdded);
    disconnect(model.get(), &DirectoryModel::fileRenamed,  this, &DirectoryPresenter::onFileRenamed);
    disconnect(model.get(), &DirectoryModel::fileModified, this, &DirectoryPresenter::onFileModified);
    disconnect(model.get(), &DirectoryModel::dirRemoved,   this, &DirectoryPresenter::onDirRemoved);
    disconnect(model.get(), &DirectoryModel::dirAdded,     this, &DirectoryPresenter::onDirAdded);
    disconnect(model.get(), &DirectoryModel::dirRenamed,   this, &DirectoryPresenter::onDirRenamed);
    //delete model;
    //delete view;
    //view = nullptr;
    model.reset();
    // also empty view?
}

struct StaticAssertHack {
    template <typename Ty>
    static constexpr bool always_false = false;

    template <typename Ty = int>
    static void InvalidQSizeType()
    {
        static_assert(always_false<Ty>, "Your definition of qsizetype is unexpected and unknown.");
    }
};

void DirectoryPresenter::setView(IDirectoryView *newView)
{
    if (view)
        return;
    view = newView;
    if (model) {
        auto n = mShowDirs ? model->totalCount() : model->fileCount();
        view->populate(n);
    }

    qRegisterMetaType<IDirectoryView::SelectionList>("IDirectoryView::SelectionList");
#if 1
    auto *obj = dynamic_cast<QObject *>(view);
    connect(obj, SIGNAL(itemActivated(qsizetype)), this, SLOT(onItemActivated(qsizetype)));
    connect(obj, SIGNAL(draggedOut()),             this, SLOT(onDraggedOut()));
    connect(obj, SIGNAL(draggedOver(qsizetype)),   this, SLOT(onDraggedOver(qsizetype)));
    connect(obj, SIGNAL(droppedInto(const QMimeData*,QObject*,qsizetype)), this, SLOT(onDroppedInto(const QMimeData*,QObject*,qsizetype)));
    connect(obj, SIGNAL(thumbnailsRequested(IDirectoryView::SelectionList,int,bool,bool)), this, SLOT(generateThumbnails(IDirectoryView::SelectionList,int,bool,bool)));
#else
    connect(view, &IDirectoryView::itemActivated, this, &DirectoryPresenter::onItemActivated);
    connect(view, &IDirectoryView::draggedOut,    this, &DirectoryPresenter::onDraggedOut);
    connect(view, &IDirectoryView::draggedOver,   this, &DirectoryPresenter::onDraggedOver);
    connect(view, &IDirectoryView::droppedInto,   this, &DirectoryPresenter::onDroppedInto);
    connect(view, &IDirectoryView::thumbnailsRequested, this, &DirectoryPresenter::generateThumbnails);
#endif
}

void DirectoryPresenter::setModel(QSharedPointer<DirectoryModel> const &newModel)
{
    if (model)
        unsetModel();
    if (!newModel)
        return;
    model = newModel;
    populateView();

    // filesystem changes
    connect(model.get(), &DirectoryModel::fileRemoved,  this, &DirectoryPresenter::onFileRemoved);
    connect(model.get(), &DirectoryModel::fileAdded,    this, &DirectoryPresenter::onFileAdded);
    connect(model.get(), &DirectoryModel::fileRenamed,  this, &DirectoryPresenter::onFileRenamed);
    connect(model.get(), &DirectoryModel::fileModified, this, &DirectoryPresenter::onFileModified);
    connect(model.get(), &DirectoryModel::dirRemoved,   this, &DirectoryPresenter::onDirRemoved);
    connect(model.get(), &DirectoryModel::dirAdded,     this, &DirectoryPresenter::onDirAdded);
    connect(model.get(), &DirectoryModel::dirRenamed,   this, &DirectoryPresenter::onDirRenamed);
}

void DirectoryPresenter::reloadModel()
{
    populateView();
}

void DirectoryPresenter::populateView()
{
    if (!model || !view)
        return;
    view->populate(mShowDirs ? model->totalCount() : model->fileCount());
    selectAndFocus(0);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void DirectoryPresenter::disconnectView()
{
    // todo
}

//------------------------------------------------------------------------------

void DirectoryPresenter::onFileRemoved(QString const &filePath, qsizetype index) const
{
    Q_UNUSED(filePath)
    if (!view)
        return;
    view->removeItem(mShowDirs ? index + model->dirCount() : index);
}

void DirectoryPresenter::onFileRenamed(QString const &fromPath, qsizetype indexFrom, QString const &toPath, qsizetype indexTo) const
{
    Q_UNUSED(fromPath)
    Q_UNUSED(toPath)

    if (!view)
        return;
    if (mShowDirs) {
        indexFrom += model->dirCount();
        indexTo += model->dirCount();
    }
    auto oldSelection = view->selection();
    view->removeItem(indexFrom);
    view->insertItem(indexTo);
    // re-select if needed
    if (oldSelection.contains(indexFrom)) {
        if (oldSelection.count() == 1) {
            view->select(indexTo);
            view->focusOn(indexTo);
        } else if (oldSelection.count() > 1) {
            view->select(view->selection() << indexTo);
        }
    }
}

void DirectoryPresenter::onFileAdded(QString const &filePath) const
{
    if (!view)
        return;
    auto index = model->indexOfFile(filePath);
    view->insertItem(mShowDirs ? model->dirCount() + index : index);
}

void DirectoryPresenter::onFileModified(QString const &filePath) const
{
    if (!view)
        return;
    auto index = model->indexOfFile(filePath);
    view->reloadItem(mShowDirs ? model->dirCount() + index : index);
}

void DirectoryPresenter::onDirRemoved(QString const &dirPath, qsizetype index) const
{
    Q_UNUSED(dirPath)
    if (!view || !mShowDirs)
        return;
    view->removeItem(index);
}

void DirectoryPresenter::onDirRenamed(QString const &fromPath, qsizetype indexFrom, QString const &toPath, qsizetype indexTo) const
{
    Q_UNUSED(fromPath)
    Q_UNUSED(toPath)
    if (!view || !mShowDirs)
        return;
    auto oldSelection = view->selection();
    view->removeItem(indexFrom);
    view->insertItem(indexTo);
    // re-select if needed
    if (oldSelection.contains(indexFrom)) {
        if (oldSelection.count() == 1) {
            view->select(indexTo);
            view->focusOn(indexTo);
        } else if (oldSelection.count() > 1) {
            view->select(view->selection() << indexTo);
        }
    }
}

void DirectoryPresenter::onDirAdded(QString const &dirPath) const
{
    if (!view || !mShowDirs)
        return;
    auto index = model->indexOfDir(dirPath);
    view->insertItem(index);
}

bool DirectoryPresenter::showDirs() const
{
    return mShowDirs;
}

void DirectoryPresenter::setShowDirs(bool mode)
{
    if (mode == mShowDirs)
        return;
    mShowDirs = mode;
    populateView();
}

QList<QString> DirectoryPresenter::selectedPaths() const
{
    QList<QString> paths;
    if (!view)
        return paths;
    if (mShowDirs) {
        for (auto i : view->selection())
            if (i < model->dirCount())
                paths << model->dirPathAt(i);
            else
                paths << model->filePathAt(i - model->dirCount());
    } else {
        for (auto i : view->selection())
            paths << model->filePathAt(i);
    }
    return paths;
}

void DirectoryPresenter::generateThumbnails(IDirectoryView::SelectionList const &indexes, int size, bool crop, bool force)
{
    if (!view || !model)
        return;
    thumbnailer->clearTasks();
    if (!mShowDirs) {
        for (auto i : indexes)
            thumbnailer->getThumbnailAsync(model->filePathAt(i), size, crop, force);
        return;
    }
    for (auto i : indexes) {
        if (i < model->dirCount()) {
            // tmp ------------------------------------------------------------
            // gen thumb for a directory
            // TODO: optimize & move dir icon loading to shared res; then overlay
            // the mini-thumbs on top (similar to dolphin)
            QSvgRenderer svgRenderer;
            svgRenderer.load(u":/res/icons/common/other/folder32-scalable.svg"_s);
            qreal factor = size * 0.90 / svgRenderer.defaultSize().width();
            auto *pixmap = new QPixmap(svgRenderer.defaultSize() * factor);
            pixmap->fill(Qt::transparent);
            QPainter pixPainter(pixmap);
            svgRenderer.render(&pixPainter);
            pixPainter.end();

            ImageLib::recolor(*pixmap, settings->colorScheme().icons);

            // ^----------------------------------------------------------------

            view->setThumbnail(i, QSharedPointer<Thumbnail>(new Thumbnail(model->dirNameAt(i), u"Folder"_s, size, QSharedPointer<QPixmap>(pixmap))));
        } else {
            QString path = model->filePathAt(i - model->dirCount());
            thumbnailer->getThumbnailAsync(path, size, crop, force);
        }
    }
}

void DirectoryPresenter::onThumbnailReady(QSharedPointer<Thumbnail> const &thumb, QString const &filePath) const
{
    if (!view || !model)
        return;
    auto index = model->indexOfFile(filePath);
    if (index == -1)
        return;
    auto pos = mShowDirs ? model->dirCount() + index : index;
    view->setThumbnail(pos, thumb);
}

void DirectoryPresenter::onItemActivated(qsizetype absoluteIndex)
{
    if (!model)
        return;
    if (!mShowDirs) {
        emit fileActivated(model->filePathAt(absoluteIndex));
        return;
    }
    if (absoluteIndex < model->dirCount())
        emit dirActivated(model->dirPathAt(absoluteIndex));
    else
        emit fileActivated(model->filePathAt(absoluteIndex - model->dirCount()));
}

void DirectoryPresenter::onDraggedOut()
{
    emit draggedOut(selectedPaths());
}

void DirectoryPresenter::onDraggedOver(qsizetype index) const
{
    if (!model || view->selection().contains(index))
        return;
    if (showDirs() && index < model->dirCount())
        view->setDragHover(index);
}

void DirectoryPresenter::onDroppedInto(QMimeData const *data, QObject *source, qsizetype targetIndex)
{
    if (!data->hasUrls() || model->source() != FileListSource::DIRECTORY)
        return;

    // ignore drops into selected / current folder when we are the source of dropEvent
    if (source && (view->selection().contains(targetIndex) || targetIndex == -1))
        return;
    // ignore drops into a file
    // todo: drop into a current dir when target is a file
    if (showDirs() && targetIndex >= model->dirCount())
        return;

    // convert urls to qstrings
    QStringList pathList;
    QList<QUrl> urlList = data->urls();
    for (auto const &i : urlList)
        pathList.append(i.toLocalFile());

    // get target dir path
    QString destDir;
    if (showDirs() && targetIndex < model->dirCount())
        destDir = model->dirPathAt(targetIndex);
    if (destDir.isEmpty()) // fallback to the current dir
        destDir = model->directoryPath();
    pathList.removeAll(destDir); // remove target dir from source list

    // pass to core
    emit droppedInto(pathList, destDir);
}

void DirectoryPresenter::selectAndFocus(QString const &path) const
{
    if (!model || !view || path.isEmpty())
        return;
    if (model->containsDir(path) && showDirs()) {
        auto dirIndex = model->indexOfDir(path);
        view->select(dirIndex);
        view->focusOn(dirIndex);
    } else if (model->containsFile(path)) {
        auto fileIndex = showDirs() ? model->indexOfFile(path) + model->dirCount() : model->indexOfFile(path);
        view->select(fileIndex);
        view->focusOn(fileIndex);
    }
}

void DirectoryPresenter::selectAndFocus(qsizetype index) const
{
    if (!model || !view)
        return;
    view->select(index);
    view->focusOn(index);
}
