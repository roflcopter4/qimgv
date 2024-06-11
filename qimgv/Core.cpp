/*
 * This is the main controller of the application.
 * It creates and initializes all components, then sets up the GUI and actions.
 * Most of the communication between components goes through here.
 */

#include "Core.h"
#include <stdexcept>

#ifdef Q_OS_WINDOWS
# include <Shlwapi.h>
# include <shlobj.h>
#endif

Core::Core(QObject *parent)
    : loopSlideshow(false),
      slideshow(false),
      shuffle(false),
      folderEndAction(FolderEndAction::NOTHING),
      thumbPanelPresenter(new DirectoryPresenter(this)),
      folderViewPresenter(new DirectoryPresenter(this)),
      mDrag(nullptr),
      translator(nullptr)
{
    setParent(parent);
    loadTranslation();
    initGui();
    initComponents();
    connectComponents();
    initActions();
    readSettings();
    slideshowTimer.setSingleShot(true);
    connect(settings, &Settings::settingsChanged, this, &Core::readSettings);

    QVersionNumber lastVersion = settings->lastVersion();
    if (settings->firstRun())
        onFirstRun();
    else if (appVersion > lastVersion)
        onUpdate();
}

Core::~Core()
{
    delete mw;
}

void Core::readSettings()
{
    loopSlideshow   = settings->loopSlideshow();
    folderEndAction = settings->folderEndAction();
    slideshowTimer.setInterval(settings->slideshowInterval());
    bool showDirs = (settings->folderViewMode() == FolderViewMode::EXT_FOLDERS);
    if (folderViewPresenter->showDirs() != showDirs)
        folderViewPresenter->setShowDirs(showDirs);
    if (shuffle)
        syncRandomizer();
}

void Core::showGui() const
{
    if (mw && !mw->isVisible())
        mw->showDefault();
    // TODO: this is unreliable.
    // how to make it wait until a window is shown?
    qApp->processEvents();
    QTimer::singleShot(50, mw, SLOT(setupFullUi()));
}

// create MainWindow and all widgets
void Core::initGui()
{
    if (mw)
        throw std::logic_error("Cannot initialize main window twice.");
    mw = new MW();
    mw->hide();
}

void Core::attachModel(DirectoryModel *newModel)
{
    model = QSharedPointer<DirectoryModel>(newModel);
    thumbPanelPresenter->setModel(model);
    folderViewPresenter->setModel(model);
    bool showDirs = settings->folderViewMode() == FolderViewMode::EXT_FOLDERS;
    folderViewPresenter->setShowDirs(showDirs);
    if (shuffle)
        syncRandomizer();
}

void Core::initComponents()
{
    attachModel(new DirectoryModel(nullptr));
}

void Core::connectComponents()
{
    thumbPanelPresenter->setView(mw->getThumbnailPanel());
    connect(thumbPanelPresenter, &DirectoryPresenter::fileActivated, this, &Core::onDirectoryViewFileActivated);
    connect(thumbPanelPresenter, &DirectoryPresenter::dirActivated,  this, &Core::loadPath);

    folderViewPresenter->setView(mw->getFolderView());
    connect(folderViewPresenter, &DirectoryPresenter::fileActivated, this, &Core::onDirectoryViewFileActivated);
    connect(folderViewPresenter, &DirectoryPresenter::dirActivated,  this, &Core::loadPath);
    connect(folderViewPresenter, &DirectoryPresenter::draggedOut,    this, qOverload<QList<QString> const &>(&Core::onDraggedOut));
    connect(folderViewPresenter, &DirectoryPresenter::droppedInto,   this, qOverload<QList<QString> const &, QString const &>(&Core::movePathsTo));

    connect(scriptManager, &ScriptManager::error, mw, &MW::showError);

    connect(mw, &MW::opened,                this, &Core::loadPath);
    connect(mw, &MW::droppedIn,             this, &Core::onDropIn);
    connect(mw, &MW::copyRequested,         this, &Core::copyCurrentFile);
    connect(mw, &MW::moveRequested,         this, &Core::moveCurrentFile);
    connect(mw, &MW::copyUrlsRequested,     this, qOverload<QList<QString> const &, QString const &>(&Core::copyPathsTo));
    connect(mw, &MW::moveUrlsRequested,     this, &Core::movePathsTo);
    connect(mw, &MW::cropRequested,         this, &Core::crop);
    connect(mw, &MW::cropAndSaveRequested,  this, &Core::cropAndSave);
    connect(mw, &MW::saveAsClicked,         this, &Core::requestSavePath);
    connect(mw, &MW::saveRequested,         this, &Core::saveCurrentFile);
    connect(mw, &MW::saveAsRequested,       this, &Core::saveCurrentFileAs);
    connect(mw, &MW::resizeRequested,       this, &Core::resize);
    connect(mw, &MW::renameRequested,       this, &Core::renameCurrentSelection);
    connect(mw, &MW::sortingSelected,       this, &Core::sortBy);
    connect(mw, &MW::showFoldersChanged,    this, &Core::setFoldersDisplay);
    connect(mw, &MW::discardEditsRequested, this, &Core::discardEdits);
    connect(mw, &MW::draggedOut,            this, qOverload<>(&Core::onDraggedOut));
    connect(mw, &MW::playbackFinished,      this, &Core::onPlaybackFinished);
    connect(mw, &MW::scalingRequested,      this, &Core::scalingRequest);

    connect(model->scaler, &Scaler::scalingFinished, this, &Core::onScalingFinished);
    connect(model.get(), &DirectoryModel::fileAdded,       this, &Core::onFileAdded);
    connect(model.get(), &DirectoryModel::fileRemoved,     this, &Core::onFileRemoved);
    connect(model.get(), &DirectoryModel::fileRenamed,     this, &Core::onFileRenamed);
    connect(model.get(), &DirectoryModel::fileModified,    this, &Core::onFileModified);
    connect(model.get(), &DirectoryModel::loaded,          this, &Core::onModelLoaded);
    connect(model.get(), &DirectoryModel::imageReady,      this, &Core::onModelItemReady);
    connect(model.get(), &DirectoryModel::imageUpdated,    this, &Core::onModelItemUpdated);
    connect(model.get(), &DirectoryModel::sortingChanged,  this, &Core::onModelSortingChanged);
    connect(model.get(), &DirectoryModel::loadFailed,      this, &Core::onLoadFailed);

    connect(&slideshowTimer, &QTimer::timeout, this, &Core::nextImageSlideshow);
}

void Core::initActions()
{
    connect(actionManager, &ActionManager::nextImage,               this, &Core::nextImage);
    connect(actionManager, &ActionManager::prevImage,               this, &Core::prevImage);
    connect(actionManager, &ActionManager::fitWindow,               mw,   &MW::fitWindow);
    connect(actionManager, &ActionManager::fitWidth,                mw,   &MW::fitWidth);
    connect(actionManager, &ActionManager::fitNormal,               mw,   &MW::fitOriginal);
    connect(actionManager, &ActionManager::toggleFitMode,           mw,   &MW::switchFitMode);
    connect(actionManager, &ActionManager::toggleFullscreen,        mw,   &MW::triggerFullScreen);
    connect(actionManager, &ActionManager::lockZoom,                mw,   &MW::toggleLockZoom);
    connect(actionManager, &ActionManager::lockView,                mw,   &MW::toggleLockView);
    connect(actionManager, &ActionManager::zoomIn,                  mw,   &MW::zoomIn);
    connect(actionManager, &ActionManager::zoomOut,                 mw,   &MW::zoomOut);
    connect(actionManager, &ActionManager::zoomInCursor,            mw,   &MW::zoomInCursor);
    connect(actionManager, &ActionManager::zoomOutCursor,           mw,   &MW::zoomOutCursor);
    connect(actionManager, &ActionManager::scrollUp,                mw,   &MW::scrollUp);
    connect(actionManager, &ActionManager::scrollDown,              mw,   &MW::scrollDown);
    connect(actionManager, &ActionManager::scrollLeft,              mw,   &MW::scrollLeft);
    connect(actionManager, &ActionManager::scrollRight,             mw,   &MW::scrollRight);
    connect(actionManager, &ActionManager::resize,                  this, &Core::showResizeDialog);
    connect(actionManager, &ActionManager::flipH,                   this, &Core::flipH);
    connect(actionManager, &ActionManager::flipV,                   this, &Core::flipV);
    connect(actionManager, &ActionManager::rotateLeft,              this, &Core::rotateLeft);
    connect(actionManager, &ActionManager::rotateRight,             this, &Core::rotateRight);
    connect(actionManager, &ActionManager::openSettings,            mw,   &MW::showSettings);
    connect(actionManager, &ActionManager::crop,                    this, &Core::toggleCropPanel);
    connect(actionManager, &ActionManager::setWallpaper,            this, &Core::setWallpaper);
    connect(actionManager, &ActionManager::open,                    this, &Core::showOpenDialog);
    connect(actionManager, &ActionManager::save,                    this, &Core::saveCurrentFile);
    connect(actionManager, &ActionManager::saveAs,                  this, &Core::requestSavePath);
    connect(actionManager, &ActionManager::exit,                    this, &Core::close);
    connect(actionManager, &ActionManager::closeFullScreenOrExit,   mw,   &MW::closeFullScreenOrExit);
    connect(actionManager, &ActionManager::removeFile,              this, &Core::removePermanent);
    connect(actionManager, &ActionManager::moveToTrash,             this, &Core::moveToTrash);
    connect(actionManager, &ActionManager::copyFile,                mw,   &MW::triggerCopyOverlay);
    connect(actionManager, &ActionManager::moveFile,                mw,   &MW::triggerMoveOverlay);
    connect(actionManager, &ActionManager::jumpToFirst,             this, &Core::jumpToFirst);
    connect(actionManager, &ActionManager::jumpToLast,              this, &Core::jumpToLast);
    connect(actionManager, &ActionManager::runScript,               this, &Core::runScript);
    connect(actionManager, &ActionManager::pauseVideo,              mw,   &MW::pauseVideo);
    connect(actionManager, &ActionManager::seekVideoForward,        mw,   &MW::seekVideoForward);
    connect(actionManager, &ActionManager::seekVideoBackward,       mw,   &MW::seekVideoBackward);
    connect(actionManager, &ActionManager::frameStep,               mw,   &MW::frameStep);
    connect(actionManager, &ActionManager::frameStepBack,           mw,   &MW::frameStepBack);
    connect(actionManager, &ActionManager::folderView,              this, &Core::enableFolderView);
    connect(actionManager, &ActionManager::documentView,            this, &Core::enableDocumentView);
    connect(actionManager, &ActionManager::toggleFolderView,        this, &Core::toggleFolderView);
    connect(actionManager, &ActionManager::reloadImage,             this, qOverload<>(&Core::reloadImage));
    connect(actionManager, &ActionManager::copyFileClipboard,       this, &Core::copyFileClipboard);
    connect(actionManager, &ActionManager::copyPathClipboard,       this, &Core::copyPathClipboard);
    connect(actionManager, &ActionManager::renameFile,              this, &Core::showRenameDialog);
    connect(actionManager, &ActionManager::contextMenu,             mw,   &MW::showContextMenu);
    connect(actionManager, &ActionManager::toggleTransparencyGrid,  mw,   &MW::toggleTransparencyGrid);
    connect(actionManager, &ActionManager::sortByName,              this, &Core::sortByName);
    connect(actionManager, &ActionManager::sortByTime,              this, &Core::sortByTime);
    connect(actionManager, &ActionManager::sortBySize,              this, &Core::sortBySize);
    connect(actionManager, &ActionManager::toggleImageInfo,         mw,   &MW::toggleImageInfoOverlay);
    connect(actionManager, &ActionManager::toggleShuffle,           this, &Core::toggleShuffle);
    connect(actionManager, &ActionManager::toggleScalingFilter,     mw,   &MW::toggleScalingFilter);
    connect(actionManager, &ActionManager::showInDirectory,         this, &Core::showInDirectory);
    connect(actionManager, &ActionManager::toggleMute,              mw,   &MW::toggleMute);
    connect(actionManager, &ActionManager::volumeUp,                mw,   &MW::volumeUp);
    connect(actionManager, &ActionManager::volumeDown,              mw,   &MW::volumeDown);
    connect(actionManager, &ActionManager::toggleSlideshow,         this, &Core::toggleSlideshow);
    connect(actionManager, &ActionManager::goUp,                    this, &Core::loadParentDir);
    connect(actionManager, &ActionManager::discardEdits,            this, &Core::discardEdits);
    connect(actionManager, &ActionManager::nextDirectory,           this, &Core::nextDirectory);
    connect(actionManager, &ActionManager::prevDirectory,           this, qOverload<>(&Core::prevDirectory));
    connect(actionManager, &ActionManager::print,                   this, &Core::print);
    connect(actionManager, &ActionManager::toggleFullscreenInfoBar, this, &Core::toggleFullscreenInfoBar);
    connect(actionManager, &ActionManager::pasteFile,               this, &Core::openFromClipboard);
}

void Core::loadTranslation()
{
    if (!translator)
        translator = new QTranslator(this);
    QString trPathFallback = QCoreApplication::applicationDirPath() + QSV("/translations");
#ifdef TRANSLATIONS_PATH
    QString trPath = QStringLiteral(TRANSLATIONS_PATH);
#else
    QString const &trPath = trPathFallback;
#endif
    QString localeName = settings->language();
    if (localeName == QSV("system"))
        localeName = QLocale::system().name();
    if (localeName.isEmpty() || localeName == QSV("en_US")) {
        QApplication::removeTranslator(translator);
        return;
    }
    QString trFile         = trPath + u'/' + localeName;
    QString trFileFallback = trPathFallback + u'/' + localeName;
    if (!translator->load(trFile)) {
        qDebug() << u"Could not load translation file: " << trFile;
        if (!translator->load(trFileFallback)) {
            qDebug() << u"Could not load translation file: " << trFileFallback;
            return;
        }
    }
    QApplication::installTranslator(translator);
}

void Core::onUpdate()
{
    QVersionNumber lastVer = settings->lastVersion();

    if (lastVer < QVersionNumber(0, 9, 2)) {
        actionManager->resetDefaults(QS("print"));
        actionManager->resetDefaults(QS("openSettings"));
    }

#ifdef USE_OPENCV
    if (lastVer < QVersionNumber(0, 9, 0))
        settings->setScalingFilter(ScalingFilter::CV_CUBIC);
#endif

    actionManager->adjustFromVersion(lastVer);

    qDebug() << u"Updated: " << settings->lastVersion().toString() << u'>' << appVersion.toString();
    // TODO: finish changelogs
    // if(settings->showChangelogs())
    //    mw->showChangelogWindow();
    mw->showMessage(tr("Updated: ") + settings->lastVersion().toString() + QSV(" > ") + appVersion.toString(), 4000);
    settings->setLastVersion(appVersion);
}

void Core::onFirstRun()
{
    // mw->showSomeSortOfWelcomeScreen();
    mw->showMessage(tr("Welcome to ") + qApp->applicationName() + tr(" version ") + appVersion.toString() + u'!', 4000);
    settings->setFirstRun(false);
    settings->setLastVersion(appVersion);
}

void Core::toggleShuffle()
{
    if (shuffle) {
        mw->showMessage(tr("Shuffle mode: OFF"));
    } else {
        syncRandomizer();
        mw->showMessage(tr("Shuffle mode: ON"));
    }
    shuffle = !shuffle;
    updateInfoString();
}

void Core::toggleSlideshow()
{
    if (slideshow) {
        stopSlideshow();
        mw->showMessage(tr("Slideshow: OFF"));

    } else {
        startSlideshow();
        mw->showMessage(tr("Slideshow: ON"));
    }
}

void Core::startSlideshow()
{
    if (!slideshow) {
        slideshow = true;
        mw->setLoopPlayback(false);
        enableDocumentView();
        startSlideshowTimer();
        updateInfoString();
    }
}

void Core::stopSlideshow()
{
    if (slideshow) {
        slideshow = false;
        mw->setLoopPlayback(true);
        slideshowTimer.stop();
        updateInfoString();
    }
}

void Core::onPlaybackFinished()
{
    if (slideshow)
        nextImageSlideshow();
}

void Core::syncRandomizer()
{
    if (model) {
        randomizer.setCount(model->fileCount());
        randomizer.shuffle();
        randomizer.setCurrent(model->indexOfFile(state.currentFilePath));
    }
}

void Core::onModelLoaded()
{
    thumbPanelPresenter->reloadModel();
    folderViewPresenter->reloadModel();
    thumbPanelPresenter->selectAndFocus(state.currentFilePath);
    folderViewPresenter->selectAndFocus(state.currentFilePath);
    if (shuffle)
        syncRandomizer();
}

void Core::onDirectoryViewFileActivated(QString const &filePath)
{
    // We aren`t using async load, so it won't flicker with empty view.
    mw->enableDocumentView();
    loadPath(filePath);
}

void Core::rotateLeft()
{
    rotateByDegrees(-90);
}

void Core::rotateRight()
{
    rotateByDegrees(90);
}

void Core::close() const
{
    mw->close();
}

void Core::removePermanent()
{
    auto paths = currentSelection();
    if (!paths.count())
        return;
    if (settings->confirmDelete()) {
        QString msg;
        if (paths.count() > 1)
            msg = tr("Delete ") + QString::number(paths.count()) + tr(" items permanently?");
        else
            msg = tr("Delete item permanently?");
        if (!mw->showConfirmation(tr("Delete permanently"), msg))
            return;
    }
    FileOpResult result{};
    int          successCount = 0;
    for (auto const &path : paths) {
        QFileInfo fi(path);
        if (fi.isDir())
            model->removeDir(path, false, true, result);
        else
            result = removeFile(path, false);
        if (result == FileOpResult::SUCCESS)
            successCount++;
    }
    if (paths.count() == 1)
        if (result == FileOpResult::SUCCESS)
            mw->showMessageSuccess(tr("File removed"));
        else
            outputError(result);
    else if (paths.count() > 1)
        mw->showMessageSuccess(tr("Removed: ") + QString::number(successCount) + tr(" files"));
}

void Core::moveToTrash()
{
    auto paths = currentSelection();
    if (!paths.count())
        return;
    if (settings->confirmTrash()) {
        QString msg;
        if (paths.count() > 1)
            msg = tr("Move ") + QString::number(paths.count()) + tr(" items to trash?");
        else
            msg = tr("Move item to trash?");
        if (!mw->showConfirmation(tr("Move to trash"), msg))
            return;
    }

    int          successCount = 0;
    FileOpResult result{};
    for (auto const &path : paths) {
        result = removeFile(path, true);
        if (result == FileOpResult::SUCCESS)
            successCount++;
    }
    if (paths.count() == 1)
        if (result == FileOpResult::SUCCESS)
            mw->showMessageSuccess(tr("Moved to trash"));
        else
            outputError(result);
    else if (paths.count() > 1)
        mw->showMessageSuccess(tr("Moved to trash: ") + QString::number(successCount) + tr(" files"));
}

void Core::reloadImage()
{
    reloadImage(selectedPath());
}

void Core::reloadImage(QString const &filePath)
{
    if (model->isEmpty())
        return;
    model->reload(filePath);
}

void Core::enableFolderView()
{
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW)
        return;
    stopSlideshow();
    mw->enableFolderView();
}

void Core::enableDocumentView()
{
    if (mw->currentViewMode() == ViewMode::DOCUMENT)
        return;
    mw->enableDocumentView();
    if (model && model->fileCount() && state.currentFilePath.isEmpty()) {
        auto selected = folderViewPresenter->selectedPaths().first();
        // if it is a directory - ignore and just open the first file
        if (model->containsFile(selected))
            loadPath(selected);
        else
            loadPath(model->firstFile());
    }
}

void Core::toggleFolderView()
{
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW)
        enableDocumentView();
    else
        enableFolderView();
}

// TODO: also copy selection from folder view?
void Core::copyFileClipboard()
{
    if (model->isEmpty())
        return;

    QMimeData *mimeData = getMimeDataForImage(model->getImage(selectedPath()), MimeDataTarget::TARGET_CLIPBOARD);

    // mimeData->text() should already contain an url
    QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl(mimeData->text()).toEncoded());
    mimeData->setData(QS("x-special/gnome-copied-files"), gnomeFormat);
    mimeData->setData(QS("application/x-kde-cutselection"), "0");

    QApplication::clipboard()->setMimeData(mimeData);
    mw->showMessage(tr("File copied"));
}

void Core::copyPathClipboard()
{
    if (model->isEmpty())
        return;
    QApplication::clipboard()->setText(selectedPath());
    mw->showMessage(tr("Path copied"));
}

// open from clipboard
// todo: actual file paste into folderview (like filemanager)
void Core::openFromClipboard()
{
    auto cb       = QApplication::clipboard();
    auto mimeData = cb->mimeData();
    if (!mimeData)
        return;
    qDebug() << u"=====================================";
    qDebug() << u"hasUrls:" << mimeData->hasUrls();
    qDebug() << u"hasImage:" << mimeData->hasImage();
    qDebug() << u"hasText:" << mimeData->hasText();

    qDebug() << u"TEXT:" << cb->text();

    // try opening url
    if (mimeData->hasUrls()) {
        auto    url  = mimeData->urls().first();
        QString path = url.toLocalFile();
        if (path.isEmpty()) {
            qDebug() << u"Could not load url:" << url;
            qDebug() << u"Currently only local files are supported.";
        } else if (loadPath(path)) {
            return;
        }
    }
    // try to save buffer image then open
    if (mimeData->hasImage()) {
        auto image = cb->image();
        if (image.isNull())
            return;
        QString destPath;
        if (!model->isEmpty())
            destPath = model->directoryPath() + u'/';
        else
            destPath = QDir::homePath() + u'/';
        destPath.append(u"clipboard.png");
        destPath = mw->getSaveFileName(destPath);
        if (destPath.isEmpty())
            return;


        // ------- temporarily copypasted from ImageStatic (needs refactoring)

        QString tmpPath = destPath + u'_' + QString::fromLatin1(QCryptographicHash::hash(destPath.toUtf8(), QCryptographicHash::Md5).toHex());
        QFileInfo fi(destPath);
        QString   ext     = fi.suffix();
        int       quality = 95;
        if (ext.compare(QSV("png"), Qt::CaseInsensitive) == 0)
            quality = 30;
        else if (ext.compare(QSV("jpg"), Qt::CaseInsensitive) == 0 || ext.compare(QSV("jpeg"), Qt::CaseInsensitive) == 0)
            quality = settings->JPEGSaveQuality();

        bool backupExists = false, success = false, originalExists = false;

        if (QFile::exists(destPath))
            originalExists = true;

        // backup the original file if possible
        if (originalExists) {
            QFile::remove(tmpPath);
            if (!QFile::copy(destPath, tmpPath)) {
                qDebug() << u"Could not create file backup.";
                return;
            }
            backupExists = true;
        }
        // save file
        success = image.save(destPath, ext.toStdString().c_str(), quality);

        if (backupExists) {
            if (success) {
                // everything ok - remove the backup
                QFile file(tmpPath);
                file.remove();
            } else if (originalExists) {
                // revert on fail
                QFile::remove(destPath);
                QFile::copy(tmpPath, destPath);
                QFile::remove(tmpPath);
            }
        }
        // ------------------------------------------
        if (success)
            loadPath(destPath);
    }
}

#ifdef Q_OS_WINDOWS
static QString evilWindowsMimeDataHack(QMimeData const *mimeData)
{
    QByteArray shIdList = mimeData->data(QS("application/x-qt-windows-mime;value=\"Shell IDList Array\""));

    char const *data = shIdList.constData();
    char const *end  = data + shIdList.size();
    char const *last = data + *reinterpret_cast<uint32_t const *>(data + 8);
    data += 35;

    // The drive letter is an ASCII string only.
    //QString str = QSV(R"(\\?\)") + QString::fromLatin1(data);
    QString str = QSV(R"(\\?\)") + QString::fromLatin1(data, strnlen(data, end - data));

    // We blindly add path separators later so ensure there isn't one.
    if (str.endsWith(u'\\'))
        str.chop(1);
    data += str.size() + 1;
    if (data >= end)
        return {};
    // The data always appears to be aligned to an odd offset. So we make sure.
    if ((reinterpret_cast<uintptr_t>(data) & 1) == 0)
        ++data;
    data += 28; // Skip 32 bytes of data.

    while (data + 2 < end) {
        // ASCII short name (null-terminated)
        data += strnlen(data, end - data) + 1;
        if (data + 1 >= end)
            break;
        if ((reinterpret_cast<uintptr_t>(data) & 1) == 0)
            ++data;
        data += 46; // Skip 46 bytes of unknown data

        // UTF-16 path segment (null-terminated). Qt won't accept an unaligned string,
        // so we must copy it ourselves.
        size_t len = wcslen(reinterpret_cast<wchar_t const *>(data));
        auto   tmp = std::make_unique<wchar_t[]>(len + 1);
        memcpy(tmp.get(), data, (len + 1) * sizeof(wchar_t));
        str.append(u'\\');
        str.append(QStringView(tmp.get(), qsizetype(len)));

        // Skip UTF-16 data and null terminator
        data += (len + 1) * sizeof(wchar_t);
        // Skip 16 bytes for each entry but the last, in which case we must skip 18.
        // The `last` offset is 4 bytes forward of where we are.
        data += data + 4 == last ? 18 : 16;
    }

    return str;
}
#endif

void Core::onDropIn(QMimeData const *mimeData, QObject const *source)
{
    // Ignore self.
    if (source == this)
        return;
    // Check for our needed mime type, here a file or a list of files.
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        if (urlList.isEmpty()) {
            static constexpr char16_t commonWarning[] = u"mimeData->urls() returned an empty list despite mimeData->hasUrls() returning true.";
#ifdef Q_OS_WINDOWS
            QString path = evilWindowsMimeDataHack(mimeData);
            if (path.isEmpty()) {
                qWarning() << commonWarning << u"The backup hack failed to return any path. Ignoring event.";
            } else {
                qDebug() << commonWarning << u"Path" << path << u"found via backup hack.";
                loadPath(std::move(path));
            }
#else
            qWarning() << commonWarning << u"Ignoring event.";
#endif
        } else {
            QStringList pathList;
            // Extract the local paths of the files.
            for (auto const &i : urlList)
                pathList.append(i.toLocalFile());
            // Try to open first file in the list.
            loadPath(pathList.first());
        }
    }
}

// drag'n'drop
// drag image out of the program
void Core::onDraggedOut()
{
    onDraggedOut(currentSelection());
}

void Core::onDraggedOut(QList<QString> const &paths)
{
    if (paths.isEmpty())
        return;
    QMimeData *mimeData;
    // single selection, image
    if (paths.count() == 1 && model->containsFile(paths.first())) {
        mimeData = getMimeDataForImage(model->getImage(paths.last()), MimeDataTarget::TARGET_DROP);
    } else { // multi-selection, or single directory. drag urls
        mimeData = new QMimeData();
        QList<QUrl> urlList;
        for (auto const &path : paths)
            urlList << QUrl::fromLocalFile(path);
        mimeData->setUrls(urlList);
    }
    // auto thumb = Thumbnailer::getThumbnail(paths.last(), 100);
    mDrag = new QDrag(this);
    mDrag->setMimeData(mimeData);
    // mDrag->setPixmap(*thumb->pixmap().get());
    mDrag->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, Qt::CopyAction);
}

QMimeData *Core::getMimeDataForImage(QSharedPointer<Image> const &img, MimeDataTarget target)
{
    auto mimeData = new QMimeData();
    if (!img)
        return mimeData;
    QString path = img->filePath();
    if (img->type() == DocumentType::STATIC) {
        if (img->isEdited()) {
            // TODO: cleanup temp files
            // meanwhile use generic name
            // path = settings->cacheDir() + img->baseName() + ".png";
            path = settings->tmpDir() + QSV("image.png");
            // use faster compression for drag'n'drop
            int pngQuality = (target == MimeDataTarget::TARGET_DROP) ? 80 : 30;
            std::ignore    = img->getImage()->save(path, nullptr, pngQuality);
        }
    }
    // !!! using setImageData() while doing drag'n'drop hangs Xorg !!!
    // clipboard only!
    if (img->type() != DocumentType::VIDEO && target == MimeDataTarget::TARGET_CLIPBOARD)
        mimeData->setImageData(*img->getImage());
    mimeData->setUrls({QUrl::fromLocalFile(path)});
    return mimeData;
}

void Core::sortBy(SortingMode mode) const
{
    model->setSortingMode(mode);
}

void Core::setFoldersDisplay(bool mode)
{
    if (folderViewPresenter->showDirs() != mode)
        folderViewPresenter->setShowDirs(mode);
}

void Core::renameCurrentSelection(QString const &newName)
{
    if (!model->fileCount() || newName.isEmpty() || selectedPath().isEmpty())
        return;
    FileOpResult result;
    model->renameEntry(selectedPath(), newName, false, result);
    if (result == FileOpResult::DESTINATION_DIR_EXISTS) {
        mw->toggleRenameOverlay(newName);
    } else if (result == FileOpResult::DESTINATION_FILE_EXISTS) {
        if (mw->showConfirmation(tr("File exists"), tr("Overwrite file?"))) {
            model->renameEntry(selectedPath(), newName, true, result);
        } else {
            // show rename dialog again
            mw->toggleRenameOverlay(newName);
        }
    }
    outputError(result);
}

FileOpResult Core::removeFile(QString const &filePath, bool trash)
{
    if (model->isEmpty())
        return FileOpResult::NOTHING_TO_DO;

    bool reopen = false;
    QSharedPointer<Image> img;
    if (state.currentFilePath == filePath) {
        img = model->getImage(filePath);
        if (img->type() == DocumentType::ANIMATED || img->type() == DocumentType::VIDEO) {
            mw->closeImage();
            reopen = true;
        }
    }
    FileOpResult result;
    model->removeFile(filePath, trash, result);
    if (result != FileOpResult::SUCCESS && reopen)
        guiSetImage(img);
    return result;
}

void Core::onFileRemoved(QString const &filePath, int index)
{
    // no files left
    if (model->isEmpty()) {
        mw->closeImage();
        state.hasActiveImage  = false;
        state.currentFilePath = QString();
    }
    // image mode && removed current file
    if (state.currentFilePath == filePath) {
        if (mw->currentViewMode() == ViewMode::DOCUMENT) {
            if (!loadFileIndex(index, true, settings->usePreloader()))
                loadFileIndex(--index, true, settings->usePreloader());
        } else {
            state.hasActiveImage  = false;
            state.currentFilePath = QString();
        }
    }
    updateInfoString();
}

void Core::onFileRenamed(QString const &fromPath, int /*indexFrom*/, QString const &/*toPath*/, int indexTo)
{
    if (state.currentFilePath == fromPath)
        loadFileIndex(indexTo, true, settings->usePreloader());
}

void Core::onFileAdded(QString const &filePath)
{
    Q_UNUSED(filePath)
    // update file count
    updateInfoString();
    if (model->fileCount() == 1 && state.currentFilePath.isEmpty())
        loadFileIndex(0, false, settings->usePreloader());
}

// !! fixme
void Core::onFileModified(QString const &filePath)
{
    (void)this;
    Q_UNUSED(filePath)
}

void Core::outputError(FileOpResult const &error) const
{
    if (error == FileOpResult::SUCCESS || error == FileOpResult::NOTHING_TO_DO)
        return;
    mw->showError(FileOperations::decodeResult(error));
    qDebug() << FileOperations::decodeResult(error);
}

void Core::showOpenDialog() const
{
    mw->showOpenDialog(model->directoryPath());
}

void Core::showInDirectory()
{
    if (!model)
        return;
    if (selectedPath().isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(model->directoryPath()));
        return;
    }
#if defined Q_OS_LINUX
    QString fm = ScriptManager::runCommand(QS("xdg-mime query default inode/directory"));
    if (fm.contains(QSV("dolphin")))
        ScriptManager::runCommandDetached(QSV("dolphin --select ") + selectedPath());
    else if (fm.contains(QSV("nautilus")))
        ScriptManager::runCommandDetached(QSV("nautilus --select ") + selectedPath());
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(model->directoryPath()));
#elif defined Q_OS_WIN32
    QStringList    args;
    args << QS("/select,") << QDir::toNativeSeparators(selectedPath());
    QProcess::startDetached(QS("explorer"), args);
#elif defined Q_OS_APPLE
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + selectedPath() + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(model->directoryPath()));
#endif
}

void Core::interactiveCopy(QList<QString> const &paths, QString const &destDirectory)
{
    DialogResult overwriteFiles;
    for (auto const &path : paths) {
        doInteractiveCopy(path, destDirectory, overwriteFiles);
        if (overwriteFiles.cancel)
            return;
    }
}

// todo: replacing DIR with a FILE?
void Core::doInteractiveCopy(QString const &path, QString const &destDirectory, DialogResult &overwriteFiles)
{
    QFileInfo srcFi(path);
    // SINGLE FILE COPY ===========================================================================
    if (!srcFi.isDir()) {
        FileOpResult result;
        FileOperations::copyFileTo(path, destDirectory, overwriteFiles, result);
        if (result == FileOpResult::DESTINATION_FILE_EXISTS) {
            if (overwriteFiles.all) // skipping all
                return;
            overwriteFiles = mw->fileReplaceDialog(
                srcFi.absoluteFilePath(), destDirectory + u'/' + srcFi.fileName(), FileReplaceMode::FILE_TO_FILE, true);
            if (!overwriteFiles || overwriteFiles.cancel)
                return;
            FileOperations::copyFileTo(path, destDirectory, true, result);
        }
        if (result != FileOpResult::SUCCESS &&
            !(result == FileOpResult::DESTINATION_FILE_EXISTS && !overwriteFiles)) {
            mw->showError(FileOperations::decodeResult(result));
            qDebug() << FileOperations::decodeResult(result);
        }
        if (!overwriteFiles.all) // copy attempt done; reset temporary flag
            overwriteFiles.yes = false;
        return;
    }
    // DIR COPY (RECURSIVE) =======================================================================
    QDir      srcDir(srcFi.absoluteFilePath());
    QFileInfo dstFi(destDirectory + u'/' + srcFi.baseName());
    QDir      dstDir(dstFi.absoluteFilePath());
    if (dstFi.exists() && !dstFi.isDir()) { // overwriting file with a folder
        if (!overwriteFiles && !overwriteFiles.all) {
            overwriteFiles = mw->fileReplaceDialog(srcFi.absoluteFilePath(), dstFi.absoluteFilePath(),
                                                   FileReplaceMode::DIR_TO_FILE, true);
            if (!overwriteFiles || overwriteFiles.cancel)
                return;
            if (!overwriteFiles.all) // reset temp flag right away
                overwriteFiles.yes = false;
        }
        // remove dst file; give up if not writable
        FileOpResult result;
        FileOperations::removeFile(dstFi.absoluteFilePath(), result);
        if (result != FileOpResult::SUCCESS) {
            mw->showError(FileOperations::decodeResult(result));
            qDebug() << FileOperations::decodeResult(result);
            return;
        }
    } else if (!dstDir.mkpath(QS("."))) {
        mw->showError(tr("Could not create directory ") + dstDir.absolutePath());
        qDebug() << u"Could not create directory " << dstDir.absolutePath();
        return;
    }
    // copy all contents
    // TODO: skip symlinks? test
    QStringList entryList = srcDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden |
                                             QDir::System);
    for (auto const &entry : entryList) {
        doInteractiveCopy(srcDir.absolutePath() + u'/' + entry, dstDir.absolutePath(), overwriteFiles);
        if (overwriteFiles.cancel)
            return;
    }
}
// -----------------------------------------------------------------------------------

void Core::interactiveMove(QList<QString> const &paths, QString const &destDirectory)
{
    DialogResult overwriteFiles;
    for (auto const &path : paths) {
        doInteractiveMove(path, destDirectory, overwriteFiles);
        if (overwriteFiles.cancel)
            return;
    }
}

// todo: replacing DIR with a FILE?
void Core::doInteractiveMove(QString const &path, QString const &destDirectory, DialogResult &overwriteFiles)
{
    QFileInfo srcFi(path);
    // SINGLE FILE MOVE ===========================================================================
    if (!srcFi.isDir()) {
        FileOpResult result;
        model->moveFileTo(path, destDirectory, overwriteFiles, result);
        if (result == FileOpResult::DESTINATION_FILE_EXISTS) {
            if (overwriteFiles.all) // skipping all
                return;
            overwriteFiles = mw->fileReplaceDialog(
                srcFi.absoluteFilePath(), destDirectory + u'/' + srcFi.fileName(), FileReplaceMode::FILE_TO_FILE, true);
            if (!overwriteFiles || overwriteFiles.cancel)
                return;
            model->moveFileTo(path, destDirectory, true, result);
        }
        if (result != FileOpResult::SUCCESS &&
            !(result == FileOpResult::DESTINATION_FILE_EXISTS && !overwriteFiles)) {
            mw->showError(FileOperations::decodeResult(result));
            qDebug() << FileOperations::decodeResult(result);
        }
        if (!overwriteFiles.all) // move attempt done; reset temporary flag
            overwriteFiles.yes = false;
        return;
    }
    // DIR MOVE (RECURSIVE) =======================================================================
    QDir      srcDir(srcFi.absoluteFilePath());
    QFileInfo dstFi(destDirectory + u'/' + srcFi.baseName());
    QDir      dstDir(dstFi.absoluteFilePath());
    if (dstFi.exists() && !dstFi.isDir()) { // overwriting file with a folder
        if (!overwriteFiles && !overwriteFiles.all) {
            overwriteFiles = mw->fileReplaceDialog(srcFi.absoluteFilePath(), dstFi.absoluteFilePath(),
                                                   FileReplaceMode::DIR_TO_FILE, true);
            if (!overwriteFiles || overwriteFiles.cancel)
                return;
            if (!overwriteFiles.all) // reset temp flag right away
                overwriteFiles.yes = false;
        }
        // remove dst file; give up if not writable
        FileOpResult result;
        FileOperations::removeFile(dstFi.absoluteFilePath(), result);
        if (result != FileOpResult::SUCCESS) {
            mw->showError(FileOperations::decodeResult(result));
            qDebug() << FileOperations::decodeResult(result);
            return;
        }
    } else if (!dstDir.mkpath(QS("."))) {
        mw->showError(tr("Could not create directory ") + dstDir.absolutePath());
        qDebug() << u"Could not create directory " << dstDir.absolutePath();
        return;
    }
    // move all contents
    // TODO: skip symlinks? test
    QStringList entryList = srcDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    for (auto const &entry : entryList) {
        doInteractiveMove(srcDir.absolutePath() + u'/' + entry, dstDir.absolutePath(), overwriteFiles);
        if (overwriteFiles.cancel)
            return;
    }
    FileOpResult dirRmRes;
    model->removeDir(srcDir.absolutePath(), false, false, dirRmRes);
}

// -----------------------------------------------------------------------------------

void Core::copyPathsTo(QList<QString> const &paths, QString const &destDirectory)
{
    interactiveCopy(paths, destDirectory);
}

void Core::movePathsTo(QList<QString> const &paths, QString const &destDirectory)
{
    interactiveMove(paths, destDirectory);
}

void Core::moveCurrentFile(QString const &destDirectory)
{
    if (model->isEmpty())
        return;
    // pause updates to avoid flicker
    mw->setUpdatesEnabled(false);
    // move fails during file playback, so we close it temporarily
    mw->closeImage();
    FileOpResult result;
    model->moveFileTo(selectedPath(), destDirectory, false, result);
    if (result == FileOpResult::SUCCESS) {
        mw->showMessageSuccess(tr("File moved."));
    } else if (result == FileOpResult::DESTINATION_FILE_EXISTS) {
        if (mw->showConfirmation(tr("File exists"), tr("Destination file exists. Overwrite?")))
            model->moveFileTo(selectedPath(), destDirectory, true, result);
    }
    if (result != FileOpResult::SUCCESS) {
        guiSetImage(model->getImage(selectedPath()));
        updateInfoString();
        if (result != FileOpResult::DESTINATION_FILE_EXISTS)
            outputError(result);
    }
    mw->setUpdatesEnabled(true);
    mw->repaint();
}

void Core::copyCurrentFile(QString const &destDirectory)
{
    if (model->isEmpty())
        return;
    FileOpResult result;
    model->copyFileTo(selectedPath(), destDirectory, false, result);
    if (result == FileOpResult::SUCCESS) {
        mw->showMessageSuccess(tr("File copied."));
    } else if (result == FileOpResult::DESTINATION_FILE_EXISTS) {
        if (mw->showConfirmation(tr("File exists"), tr("Destination file exists. Overwrite?")))
            model->copyFileTo(selectedPath(), destDirectory, true, result);
    }
    if (result != FileOpResult::SUCCESS && result != FileOpResult::DESTINATION_FILE_EXISTS)
        outputError(result);
}

void Core::toggleCropPanel()
{
    if (model->isEmpty())
        return;
    if (mw->isCropPanelActive())
        mw->triggerCropPanel();
    else if (state.hasActiveImage)
        mw->triggerCropPanel();
}

void Core::toggleFullscreenInfoBar()
{
    mw->toggleFullscreenInfoBar();
}

void Core::requestSavePath()
{
    if (model->isEmpty())
        return;
    mw->showSaveDialog(selectedPath());
}

void Core::showResizeDialog()
{
    if (model->isEmpty())
        return;
    if (auto const &img = model->getImage(selectedPath()))
        mw->showResizeDialog(img->size());
}

// ---------------------------------------------------------------- image operations

QSharedPointer<ImageStatic> Core::getEditableImage(QString const &filePath) const
{
    return qSharedPointerDynamicCast<ImageStatic>(model->getImage(filePath));
}

template <typename... Args>
void Core::edit_template(
    bool save,
    QString const &actionName,
    std::function<QImage *(QSharedPointer<QImage const>, Args...)> const &editFunc,
    Args &&...as)
{
    if (model->isEmpty())
        return;
    if (save && !mw->showConfirmation(actionName, tr("Perform action \"") + actionName + QSV("\"? \n\n") +
                                                  tr("Changes will be saved immediately.")))
        return;
    for (auto const &path : currentSelection()) {
        auto img = getEditableImage(path);
        if (!img)
            continue;
        img->setEditedImage(
            std::unique_ptr<QImage const>(editFunc(img->getImage(), std::forward<Args>(as)...)));
        model->updateImage(path, qSharedPointerCast<Image>(img));
        if (save) {
            saveFile(path);
            if (state.currentFilePath != path)
                model->unload(path);
        }
    }
    updateInfoString();
}

void Core::flipH()
{
    edit_template((mw->currentViewMode() == ViewMode::FOLDERVIEW), tr("Flip horizontal"), {ImageLib::flippedH});
}

void Core::flipV()
{
    edit_template((mw->currentViewMode() == ViewMode::FOLDERVIEW), tr("Flip vertical"), {ImageLib::flippedV});
}

void Core::rotateByDegrees(int degrees)
{
    edit_template((mw->currentViewMode() == ViewMode::FOLDERVIEW), tr("Rotate"), {ImageLib::rotated}, degrees);
}

void Core::resize(QSize size)
{
    edit_template(false, tr("Resize"), {ImageLib::scaled}, size, ScalingFilter::BILINEAR);
}

void Core::crop(QRect rect)
{
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW)
        return;
    edit_template(false, tr("Crop"), {ImageLib::cropped}, rect);
}

void Core::cropAndSave(QRect rect)
{
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW)
        return;
    edit_template(false, tr("Crop"), {ImageLib::cropped}, rect);
    saveFile(selectedPath());
    updateInfoString();
}

// ---------------------------------------------------------------- image operations ^

bool Core::saveFile(QString const &filePath)
{
    return saveFile(filePath, filePath);
}

bool Core::saveFile(QString const &filePath, QString const &newPath)
{
    if (!model->saveFile(filePath, newPath))
        return false;
    mw->hideSaveOverlay();
    // switch to the new file
    if (model->containsFile(newPath) && state.currentFilePath != newPath) {
        discardEdits();
        if (mw->currentViewMode() == ViewMode::DOCUMENT)
            loadPath(newPath);
    }
    return true;
}

void Core::saveCurrentFile()
{
    saveCurrentFileAs(selectedPath());
}

void Core::saveCurrentFileAs(QString const &destPath)
{
    if (model->isEmpty())
        return;
    if (saveFile(selectedPath(), destPath)) {
        mw->showMessageSuccess(tr("File saved"));
        updateInfoString();
    } else {
        mw->showError(tr("Could not save file"));
    }
}

void Core::discardEdits()
{
    if (model->isEmpty())
        return;

    QSharedPointer<Image> img = model->getImage(selectedPath());
    if (img && img->type() == DocumentType::STATIC) {
        auto imgStatic = dynamic_cast<ImageStatic *>(img.get());
        imgStatic->discardEditedImage();
        model->updateImage(selectedPath(), img);
    }
    mw->hideSaveOverlay();
}

// todo: remove?
QString Core::selectedPath()
{
    if (!model)
        return {};
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW)
        return folderViewPresenter->selectedPaths().last();
    return state.currentFilePath;
}

QList<QString> Core::currentSelection() const
{
    if (!model)
        return {};
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW)
        return folderViewPresenter->selectedPaths();
    return QList<QString>() << state.currentFilePath;
}

//------------------------

void Core::sortByName()
{
    auto mode = SortingMode::NAME;
    if (model->sortingMode() == mode)
        mode = SortingMode::NAME_DESC;
    model->setSortingMode(mode);
}

void Core::sortByTime()
{
    auto mode = SortingMode::TIME;
    if (model->sortingMode() == mode)
        mode = SortingMode::TIME_DESC;
    model->setSortingMode(mode);
}

void Core::sortBySize()
{
    auto mode = SortingMode::SIZE;
    if (model->sortingMode() == mode)
        mode = SortingMode::SIZE_DESC;
    model->setSortingMode(mode);
}

void Core::showRenameDialog()
{
    if (model->isEmpty())
        return;
    QFileInfo fi(selectedPath());
    mw->toggleRenameOverlay(fi.fileName());
}

void Core::runScript(QString const &scriptName)
{
    if (model->isEmpty())
        return;
    scriptManager->runScript(scriptName, model->getImage(selectedPath()));
}

void Core::setWallpaper()
{
    if (model->isEmpty() || selectedPath().isEmpty())
        return;
    auto img = model->getImage(selectedPath());
    if (img->type() != DocumentType::STATIC) {
        mw->showMessage(QS("Set wallpaper: file not supported"));
        return;
    }
#ifdef Q_OS_WIN32
    // Set fit mode (registry).
    HKEY hKey;
    LONG status = RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKey);
    if (status == ERROR_SUCCESS && hKey != nullptr) {
        static constexpr wchar_t data[]  = L"10";
        status = RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, reinterpret_cast<BYTE const *>(data), sizeof data);
        RegCloseKey(hKey);
    }
    // Set wallpaper path.
    SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0,
                          const_cast<void *>(static_cast<void const *>(selectedPath().toStdWString().c_str())),
                          SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#else
    auto session = qgetenv("DESKTOP_SESSION").toLower();
    if (session.contains("plasma"))
        ScriptManager::runCommand(QSV("plasma-apply-wallpaperimage \"") + selectedPath() + QSV("\""));
    else if (session.contains("gnome"))
        ScriptManager::runCommand(QSV("gsettings set org.gnome.desktop.background picture-uri \"") + selectedPath() + QSV("\""));
    else
        mw->showMessage(QSV("Action is not supported in your desktop session (\"") + QString::fromUtf8(session) + QSV("\")"), 3000);
#endif
}

void Core::print()
{
    if (model->isEmpty())
        return;
    PrintDialog p(mw);
    auto        img = model->getImage(selectedPath());
    if (!img) {
        mw->showError(tr("Could not open image"));
        return;
    }
    if (img->type() != DocumentType::STATIC) {
        mw->showError(tr("Can only print static images"));
        return;
    }
    QString pdfPath = model->directoryPath() + u'/' + img->baseName() + QSV(".pdf");
    p.setImage(img->getImage());
    p.setOutputPath(pdfPath);
    p.exec();
}

void Core::scalingRequest(QSize size, ScalingFilter filter) const
{
    // filter out an unnecessary scale request at statup
    if (mw->isVisible() && state.hasActiveImage) {
        QSharedPointer<Image> forScale = model->getImage(state.currentFilePath);
        if (forScale)
            model->scaler->requestScaled(ScalerRequest(forScale, size, state.currentFilePath, filter));
    }
}

// TODO: don't use connect? otherwise there is no point using unique_ptr
void Core::onScalingFinished(QPixmap *scaled, ScalerRequest const &req)
{
    if (state.hasActiveImage /* TODO: a better fix > */ && req.path == state.currentFilePath)
        mw->onScalingFinished(std::unique_ptr<QPixmap>(scaled));
    else
        delete scaled;
}

// reset state; clear cache; etc
void Core::reset()
{
    state.hasActiveImage  = false;
    state.currentFilePath = QString{};
    model->setDirectory(QString{});
}

bool Core::loadPath(QString path)
{
    if (path.isEmpty())
        return false;
    if (path.startsWith(QSV("file://"), Qt::CaseInsensitive))
        path.remove(0, 7);

    stopSlideshow();
    state.delayModel = false;

    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        state.directoryPath = QDir(path).absolutePath();
    } else if (fileInfo.isFile()) {
        state.directoryPath = fileInfo.absolutePath();
        if (model->directoryPath() != state.directoryPath)
            state.delayModel = true;
    } else {
        mw->showError(tr("Could not open path: ") + path);
        qDebug() << u"Could not open path: " << path;
        return false;
    }

    if (!state.delayModel && !setDirectory(state.directoryPath))
        return false;

    // load file / folderview
    if (fileInfo.isFile()) {
        int index = model->indexOfFile(fileInfo.absoluteFilePath());
        // DirectoryManager only checks file extensions via regex (performance reasons)
        // But in this case we force check mimetype
        if (index == -1) {
            QStringList   types = settings->supportedMimeTypes();
            QMimeDatabase db;
            QMimeType     type = db.mimeTypeForFile(fileInfo.absoluteFilePath());
            if (types.contains(type.name())) {
                if (model->forceInsert(fileInfo.absoluteFilePath()))
                    index = model->indexOfFile(fileInfo.absoluteFilePath());
            }
        }
        mw->enableDocumentView();
        return loadFileIndex(index, false, settings->usePreloader());
    } else {
        mw->enableFolderView();
        return true;
    }
}

bool Core::setDirectory(QString const &path)
{
    if (model->directoryPath() != path) {
        reset();
        if (!model->setDirectory(path)) {
            mw->showError(tr("Could not load folder: ") + path);
            return false;
        }
        mw->setDirectoryPath(path);
    }
    return true;
}

bool Core::loadFileIndex(int index, bool async, bool preload)
{
    if (!model)
        return false;
    auto entry = model->fileEntryAt(index);
    if (entry.path.isEmpty())
        return false;
    state.currentFilePath = entry.path;
    model->unloadExcept(entry.path, preload);
    model->load(entry.path, async);
    if (preload) {
        model->preload(model->nextOf(entry.path));
        model->preload(model->prevOf(entry.path));
    }
    thumbPanelPresenter->selectAndFocus(entry.path);
    folderViewPresenter->selectAndFocus(entry.path);
    updateInfoString();
    return true;
}

void Core::loadParentDir()
{
    if (model->directoryPath().isEmpty() || mw->currentViewMode() != ViewMode::FOLDERVIEW)
        return;
    stopSlideshow();
    QFileInfo currentDir(model->directoryPath());
    QFileInfo parentDir(currentDir.absolutePath());
    if (parentDir.exists() && parentDir.isReadable())
        loadPath(parentDir.absoluteFilePath());
    folderViewPresenter->selectAndFocus(currentDir.absoluteFilePath());
}

void Core::nextDirectory()
{
    if (model->directoryPath().isEmpty() || mw->currentViewMode() != ViewMode::DOCUMENT)
        return;
    stopSlideshow();
    QFileInfo currentDir(model->directoryPath());
    QFileInfo parentDir(currentDir.absolutePath());

    if (parentDir.exists() && parentDir.isReadable()) {
        auto dm = DirectoryManager(nullptr);
        if (!dm.setDirectory(parentDir.absoluteFilePath()))
            return;

        QString next = dm.nextOfDir(model->directoryPath());
        if (!next.isEmpty()) {
            if (!setDirectory(next))
                return;
            QFileInfo fi(next);
            mw->showMessageDirectory(fi.baseName());
            if (model->fileCount())
                loadFileIndex(0, false, true);
        } else {
            mw->showMessageDirectoryEnd();
        }
    }
}

void Core::prevDirectory(bool selectLast)
{
    if (model->directoryPath().isEmpty() || mw->currentViewMode() != ViewMode::DOCUMENT)
        return;
    QFileInfo currentDir(model->directoryPath());
    QFileInfo parentDir(currentDir.absolutePath());

    if (parentDir.exists() && parentDir.isReadable()) {
        auto dm = DirectoryManager(nullptr);
        dm.setDirectory(parentDir.absoluteFilePath());

        QString prev = dm.prevOfDir(model->directoryPath());
        if (!prev.isEmpty()) {
            if (!setDirectory(prev))
                return;
            QFileInfo fi(prev);
            mw->showMessageDirectory(fi.baseName());
            if (model->fileCount()) {
                if (selectLast)
                    loadFileIndex(model->fileCount() - 1, false, true);
                else
                    loadFileIndex(0, false, true);
            }
        } else {
            mw->showMessageDirectoryStart();
        }
    }
}

void Core::prevDirectory()
{
    prevDirectory(false);
}

void Core::nextImage()
{
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW ||
        (model->isEmpty() && folderEndAction != FolderEndAction::GOTO_ADJACENT))
        return;
    stopSlideshow();
    if (shuffle) {
        loadFileIndex(randomizer.next(), true, false);
        return;
    }
    auto newIndex = model->indexOfFile(state.currentFilePath) + 1;
    if (newIndex >= model->fileCount()) {
        if (folderEndAction == FolderEndAction::LOOP) {
            newIndex = 0;
        } else if (folderEndAction == FolderEndAction::GOTO_ADJACENT) {
            nextDirectory();
            return;
        } else {
            if (!model->loaderBusy())
                mw->showMessageDirectoryEnd();
            return;
        }
    }
    loadFileIndex(newIndex, true, settings->usePreloader());
}

void Core::prevImage()
{
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW ||
        (model->isEmpty() && folderEndAction != FolderEndAction::GOTO_ADJACENT))
        return;
    stopSlideshow();
    if (shuffle) {
        loadFileIndex(randomizer.prev(), true, false);
        return;
    }

    int newIndex = model->indexOfFile(state.currentFilePath) - 1;
    if (newIndex < 0) {
        if (folderEndAction == FolderEndAction::LOOP) {
            newIndex = model->fileCount() - 1;
        } else if (folderEndAction == FolderEndAction::GOTO_ADJACENT) {
            prevDirectory(true);
            return;
        } else {
            if (!model->loaderBusy())
                mw->showMessageDirectoryStart();
            return;
        }
    }
    loadFileIndex(newIndex, true, settings->usePreloader());
}

void Core::nextImageSlideshow()
{
    if (model->isEmpty() || mw->currentViewMode() == ViewMode::FOLDERVIEW)
        return;
    if (shuffle) {
        loadFileIndex(randomizer.next(), false, false);
    } else {
        int newIndex = model->indexOfFile(state.currentFilePath) + 1;
        if (newIndex >= model->fileCount()) {
            if (loopSlideshow) {
                newIndex = 0;
            } else {
                stopSlideshow();
                mw->showMessage(tr("End of directory."));
                return;
            }
        }
        loadFileIndex(newIndex, false, true);
    }
    startSlideshowTimer();
}

void Core::startSlideshowTimer()
{
    // start timer only for static images or single frame gifs
    // for proper gifs and video we get a playbackFinished() signal
    auto img = model->getImage(state.currentFilePath);
    if (img->type() == DocumentType::STATIC) {
        slideshowTimer.start();
    } else if (img->type() == DocumentType::ANIMATED) {
        auto anim = dynamic_cast<ImageAnimated *>(img.get());
        if (anim && anim->frameCount() <= 1)
            slideshowTimer.start();
    }
}

void Core::jumpToFirst()
{
    if (model->isEmpty())
        return;
    stopSlideshow();
    loadFileIndex(0, true, settings->usePreloader());
    mw->showMessageDirectoryStart();
}

void Core::jumpToLast()
{
    if (model->isEmpty())
        return;
    stopSlideshow();
    loadFileIndex(model->fileCount() - 1, true, settings->usePreloader());
    mw->showMessageDirectoryEnd();
}

void Core::onLoadFailed(QString const &path) const
{
    mw->showMessage(tr("Load failed: ") + path);
    if (path == state.currentFilePath)
        mw->closeImage();
}

void Core::onModelItemReady(QSharedPointer<Image> const &img, QString const &path)
{
    if (path == state.currentFilePath) {
        state.currentImg = img;
        guiSetImage(img);
        updateInfoString();
        if (state.delayModel) {
            showGui();
            state.delayModel = false;
            QTimer::singleShot(40, this, SLOT(modelDelayLoad()));
        }
        model->unloadExcept(state.currentFilePath, settings->usePreloader());
    }
}

void Core::modelDelayLoad()
{
    model->setDirectory(state.directoryPath);
    mw->setDirectoryPath(state.directoryPath);
    model->updateImage(state.currentFilePath, state.currentImg);
    updateInfoString();
}

void Core::onModelItemUpdated(QString const &filePath)
{
    if (filePath == state.currentFilePath) {
        guiSetImage(model->getImage(filePath));
        updateInfoString();
    }
}

void Core::onModelSortingChanged(SortingMode mode)
{
    mw->onSortingChanged(mode);
    thumbPanelPresenter->reloadModel();
    thumbPanelPresenter->selectAndFocus(state.currentFilePath);
    folderViewPresenter->reloadModel();
    folderViewPresenter->selectAndFocus(state.currentFilePath);
}

void Core::guiSetImage(QSharedPointer<Image> const &img)
{
    state.hasActiveImage = true;
    if (!img) {
        mw->showMessage(tr("Error: could not load image."));
        return;
    }
    DocumentType type = img->type();
    if (type == DocumentType::STATIC) {
        mw->showImage(img->getPixmap());
    } else if (type == DocumentType::ANIMATED) {
        auto animated = dynamic_cast<ImageAnimated *>(img.get());
        mw->showAnimation(animated->getMovie());
    } else if (type == DocumentType::VIDEO) {
        auto video = dynamic_cast<Video *>(img.get());
        // workaround for mpv. If we play video while mainwindow is hidden we get black screen.
        // affects only initial startup (e.g. we open webm from file manager)
        showGui();
        mw->showVideo(video->filePath());
    }
    img->isEdited() ? mw->showSaveOverlay() : mw->hideSaveOverlay();
    mw->setExifInfo(img->getExifTags());
}

void Core::updateInfoString()
{
    QSize  imageSize(0, 0);
    qint64 fileSize = 0;
    bool   edited   = false;

    if (model->isLoaded(state.currentFilePath)) {
        auto img  = model->getImage(state.currentFilePath);
        imageSize = img->size();
        fileSize  = img->fileSize();
        edited    = img->isEdited();
    }
    auto index = model->indexOfFile(state.currentFilePath);
    mw->setCurrentInfo(index,
                       model->fileCount(),
                       model->filePathAt(index),
                       model->fileNameAt(index),
                       imageSize, fileSize, slideshow, shuffle, edited);
}
