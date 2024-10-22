/*
 * This is the main controller of the application.
 * It creates and initializes all components, then sets up the GUI and actions.
 * Most of the communication between components goes through here.
 */

#include "Core.h"
#ifdef Q_OS_WINDOWS
# include <ShlObj.h>
# include <Shlwapi.h>
#endif

Core::Core(QObject *parent)
    : QObject(parent),
      mw(new MW(nullptr)),
      model(QSharedPointer<DirectoryModel>(new DirectoryModel(nullptr))),
      thumbPanelPresenter(new DirectoryPresenter(this)),
      folderViewPresenter(new DirectoryPresenter(this)),
      translator(new QTranslator(this)),
      folderEndAction(settings->folderEndAction()),
      loopSlideshow(settings->loopSlideshow()),
      slideshow(false),
      shuffle(false)
{
    mw->hide();
    loadTranslation();

    thumbPanelPresenter->setModel(model);
    folderViewPresenter->setModel(model);
    folderViewPresenter->setShowDirs(settings->folderViewMode() == FolderViewMode::EXT_FOLDERS);

    connectComponents();
    connectActions();
    connect(settings, &Settings::settingsChanged, this, &Core::updateSettings);

    slideshowTimer.setInterval(settings->slideshowInterval());
    slideshowTimer.setSingleShot(true);

    // Print a welcome on the first run, or the changelog if we've updated.
    QVersionNumber lastVersion = settings->lastVersion();
    if (settings->firstRun())
        onFirstRun();
    else if (appVersion > lastVersion)
        onUpdate();
}

Core::~Core() = default;

/****************************************************************************************/

void Core::loadTranslation()
{
    QString trPathFallback = QCoreApplication::applicationDirPath() + u"/translations";
#ifdef TRANSLATIONS_PATH
    QString trPath = QStringLiteral(TRANSLATIONS_PATH);
#else
    QString const &trPath = trPathFallback;
#endif

    QString localeName = settings->language();
    if (localeName == u"system"_sv)
        localeName = QLocale::system().name();
    if (localeName.isEmpty() || localeName == u"en_US"_sv) {
        QApplication::removeTranslator(translator);
        return;
    }

    QString trFile = trPath + u'/' + localeName;
    if (!translator->load(trFile)) {
        qDebug() << u"Could not load translation file:" << trFile;
        QString trFileFallback = trPathFallback + u'/' + localeName;
        if (!translator->load(trFileFallback)) {
            qDebug() << u"Could not load translation file:" << trFileFallback;
            return;
        }
    }

    QCoreApplication::installTranslator(translator);
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

    connect(scriptManager, &ScriptManager::error, mw.get(), &MW::showError);

    connect(mw.get(), &MW::copyRequested,         this, &Core::copyCurrentFile);
    connect(mw.get(), &MW::copyUrlsRequested,     this, qOverload<QList<QString> const &, QString const &>(&Core::copyPathsTo));
    connect(mw.get(), &MW::cropAndSaveRequested,  this, &Core::cropAndSave);
    connect(mw.get(), &MW::cropRequested,         this, &Core::crop);
    connect(mw.get(), &MW::discardEditsRequested, this, &Core::discardEdits);
    connect(mw.get(), &MW::draggedOut,            this, qOverload<>(&Core::onDraggedOut));
    connect(mw.get(), &MW::droppedIn,             this, &Core::onDropIn);
    connect(mw.get(), &MW::moveRequested,         this, &Core::moveCurrentFile);
    connect(mw.get(), &MW::moveUrlsRequested,     this, &Core::movePathsTo);
    connect(mw.get(), &MW::opened,                this, &Core::loadPath);
    connect(mw.get(), &MW::playbackFinished,      this, &Core::onPlaybackFinished);
    connect(mw.get(), &MW::renameRequested,       this, &Core::renameCurrentSelection);
    connect(mw.get(), &MW::resizeRequested,       this, &Core::resize);
    connect(mw.get(), &MW::saveAsClicked,         this, &Core::requestSavePath);
    connect(mw.get(), &MW::saveAsRequested,       this, &Core::saveCurrentFileAs);
    connect(mw.get(), &MW::saveRequested,         this, &Core::saveCurrentFile);
    connect(mw.get(), &MW::scalingRequested,      this, &Core::scalingRequest);
    connect(mw.get(), &MW::showFoldersChanged,    this, &Core::setFoldersDisplay);
    connect(mw.get(), &MW::sortingSelected,       this, &Core::sortBy);

    connect(model.get(), &DirectoryModel::fileAdded,      this, &Core::onFileAdded);
    connect(model.get(), &DirectoryModel::fileRemoved,    this, &Core::onFileRemoved);
    connect(model.get(), &DirectoryModel::fileRenamed,    this, &Core::onFileRenamed);
    connect(model.get(), &DirectoryModel::fileModified,   this, &Core::onFileModified);
    connect(model.get(), &DirectoryModel::loaded,         this, &Core::onModelLoaded);
    connect(model.get(), &DirectoryModel::imageReady,     this, &Core::onModelItemReady);
    connect(model.get(), &DirectoryModel::imageUpdated,   this, &Core::onModelItemUpdated);
    connect(model.get(), &DirectoryModel::sortingChanged, this, &Core::onModelSortingChanged);
    connect(model.get(), &DirectoryModel::loadFailed,     this, &Core::onLoadFailed);

    connect(model->scaler(), &Scaler::scalingFinished, this, &Core::onScalingFinished);
    connect(&slideshowTimer, &QTimer::timeout, this, &Core::nextImageSlideshow);
}

void Core::connectActions()
{
    connect(actionManager, &ActionManager::copyFileClipboard,       this, &Core::copyFileClipboard);
    connect(actionManager, &ActionManager::copyPathClipboard,       this, &Core::copyPathClipboard);
    connect(actionManager, &ActionManager::crop,                    this, &Core::toggleCropPanel);
    connect(actionManager, &ActionManager::discardEdits,            this, &Core::discardEdits);
    connect(actionManager, &ActionManager::documentView,            this, &Core::enableDocumentView);
    connect(actionManager, &ActionManager::exit,                    this, &Core::close);
    connect(actionManager, &ActionManager::flipH,                   this, &Core::flipH);
    connect(actionManager, &ActionManager::flipV,                   this, &Core::flipV);
    connect(actionManager, &ActionManager::folderView,              this, &Core::enableFolderView);
    connect(actionManager, &ActionManager::goUp,                    this, &Core::loadParentDir);
    connect(actionManager, &ActionManager::jumpToFirst,             this, &Core::jumpToFirst);
    connect(actionManager, &ActionManager::jumpToLast,              this, &Core::jumpToLast);
    connect(actionManager, &ActionManager::moveToTrash,             this, &Core::moveToTrash);
    connect(actionManager, &ActionManager::nextDirectory,           this, &Core::nextDirectory);
    connect(actionManager, &ActionManager::nextImage,               this, &Core::nextImage);
    connect(actionManager, &ActionManager::open,                    this, &Core::showOpenDialog);
    connect(actionManager, &ActionManager::pasteFile,               this, &Core::openFromClipboard);
    connect(actionManager, &ActionManager::prevDirectory,           this, qOverload<>(&Core::prevDirectory));
    connect(actionManager, &ActionManager::prevImage,               this, &Core::prevImage);
    connect(actionManager, &ActionManager::print,                   this, &Core::print);
    connect(actionManager, &ActionManager::reloadImage,             this, qOverload<>(&Core::reloadImage));
    connect(actionManager, &ActionManager::removeFile,              this, &Core::removePermanent);
    connect(actionManager, &ActionManager::renameFile,              this, &Core::showRenameDialog);
    connect(actionManager, &ActionManager::resize,                  this, &Core::showResizeDialog);
    connect(actionManager, &ActionManager::rotateLeft,              this, &Core::rotateLeft);
    connect(actionManager, &ActionManager::rotateRight,             this, &Core::rotateRight);
    connect(actionManager, &ActionManager::runScript,               this, &Core::runScript);
    connect(actionManager, &ActionManager::save,                    this, &Core::saveCurrentFile);
    connect(actionManager, &ActionManager::saveAs,                  this, &Core::requestSavePath);
    connect(actionManager, &ActionManager::setWallpaper,            this, &Core::setWallpaper);
    connect(actionManager, &ActionManager::showInDirectory,         this, &Core::showInDirectory);
    connect(actionManager, &ActionManager::sortByName,              this, &Core::sortByName);
    connect(actionManager, &ActionManager::sortBySize,              this, &Core::sortBySize);
    connect(actionManager, &ActionManager::sortByTime,              this, &Core::sortByTime);
    connect(actionManager, &ActionManager::toggleFolderView,        this, &Core::toggleFolderView);
    connect(actionManager, &ActionManager::toggleFullscreenInfoBar, this, &Core::toggleFullscreenInfoBar);
    connect(actionManager, &ActionManager::toggleShuffle,           this, &Core::toggleShuffle);
    connect(actionManager, &ActionManager::toggleSlideshow,         this, &Core::toggleSlideshow);

    connect(actionManager, &ActionManager::closeFullScreenOrExit,  mw.get(), &MW::closeFullScreenOrExit);
    connect(actionManager, &ActionManager::contextMenu,            mw.get(), &MW::showContextMenu);
    connect(actionManager, &ActionManager::copyFile,               mw.get(), &MW::triggerCopyOverlay);
    connect(actionManager, &ActionManager::fitNormal,              mw.get(), &MW::fitOriginal);
    connect(actionManager, &ActionManager::fitWidth,               mw.get(), &MW::fitWidth);
    connect(actionManager, &ActionManager::fitWindow,              mw.get(), &MW::fitWindow);
    connect(actionManager, &ActionManager::frameStep,              mw.get(), &MW::frameStep);
    connect(actionManager, &ActionManager::frameStepBack,          mw.get(), &MW::frameStepBack);
    connect(actionManager, &ActionManager::lockView,               mw.get(), &MW::toggleLockView);
    connect(actionManager, &ActionManager::lockZoom,               mw.get(), &MW::toggleLockZoom);
    connect(actionManager, &ActionManager::moveFile,               mw.get(), &MW::triggerMoveOverlay);
    connect(actionManager, &ActionManager::openSettings,           mw.get(), &MW::showSettings);
    connect(actionManager, &ActionManager::pauseVideo,             mw.get(), &MW::pauseVideo);
    connect(actionManager, &ActionManager::scrollDown,             mw.get(), &MW::scrollDown);
    connect(actionManager, &ActionManager::scrollLeft,             mw.get(), &MW::scrollLeft);
    connect(actionManager, &ActionManager::scrollRight,            mw.get(), &MW::scrollRight);
    connect(actionManager, &ActionManager::scrollUp,               mw.get(), &MW::scrollUp);
    connect(actionManager, &ActionManager::seekVideoBackward,      mw.get(), &MW::seekVideoBackward);
    connect(actionManager, &ActionManager::seekVideoForward,       mw.get(), &MW::seekVideoForward);
    connect(actionManager, &ActionManager::toggleFitMode,          mw.get(), &MW::switchFitMode);
    connect(actionManager, &ActionManager::toggleFullscreen,       mw.get(), &MW::triggerFullScreen);
    connect(actionManager, &ActionManager::toggleImageInfo,        mw.get(), &MW::toggleImageInfoOverlay);
    connect(actionManager, &ActionManager::toggleMute,             mw.get(), &MW::toggleMute);
    connect(actionManager, &ActionManager::toggleScalingFilter,    mw.get(), &MW::toggleScalingFilter);
    connect(actionManager, &ActionManager::toggleTransparencyGrid, mw.get(), &MW::toggleTransparencyGrid);
    connect(actionManager, &ActionManager::volumeDown,             mw.get(), &MW::volumeDown);
    connect(actionManager, &ActionManager::volumeUp,               mw.get(), &MW::volumeUp);
    connect(actionManager, &ActionManager::zoomIn,                 mw.get(), &MW::zoomIn);
    connect(actionManager, &ActionManager::zoomInCursor,           mw.get(), &MW::zoomInCursor);
    connect(actionManager, &ActionManager::zoomOut,                mw.get(), &MW::zoomOut);
    connect(actionManager, &ActionManager::zoomOutCursor,          mw.get(), &MW::zoomOutCursor);
}

/****************************************************************************************/

void Core::updateSettings()
{
    loopSlideshow   = settings->loopSlideshow();
    folderEndAction = settings->folderEndAction();
    slideshowTimer.setInterval(settings->slideshowInterval());
    bool showDirs = settings->folderViewMode() == FolderViewMode::EXT_FOLDERS;
    if (folderViewPresenter->showDirs() != showDirs)
        folderViewPresenter->setShowDirs(showDirs);
    if (shuffle)
        syncRandomizer();
}

void Core::showGui()
{
    std::call_once(onceFlag_, &Core::doShowGui, this);
}

void Core::doShowGui()
{
    if (mw && !mw->isVisible())
        mw->showDefault();
    // TODO: this is unreliable.
    // how to make it wait until a window is shown?
    qApp->processEvents();
    QTimer::singleShot(50ms, mw.get(), &MW::setupFullUi);
}

void Core::onUpdate()
{
    QVersionNumber lastVer = settings->lastVersion();

    if (lastVer < QVersionNumber(0, 9, 2)) {
        actionManager->resetDefaults(u"print"_s);
        actionManager->resetDefaults(u"openSettings"_s);
    }

#ifdef USE_OPENCV
    if (lastVer < QVersionNumber(0, 9, 0))
        settings->setScalingFilter(ScalingFilter::CV_CUBIC);
#endif

    actionManager->adjustFromVersion(lastVer);

    qDebug() << u"Updated:" << settings->lastVersion().toString() << u'>' << appVersion.toString();
    // TODO: finish changelogs
    // if(settings->showChangelogs())
    //    mw->showChangelogWindow();
    mw->showMessage(tr("Updated: ") + settings->lastVersion().toString() + u" > " + appVersion.toString(), 4000);
    settings->setLastVersion(appVersion);
}

void Core::onFirstRun()
{
    //mw->showSomeSortOfWelcomeScreen();
    mw->showMessage(tr("Welcome to ") + qApp->applicationName() + tr(" version ") + appVersion.toString() + u'!', 4000);
    settings->setFirstRun(false);
    settings->setLastVersion(appVersion);
}

/****************************************************************************************/

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

void Core::close()
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
    unsigned     successCount = 0;
    for (auto const &path : paths) {
        QFileInfo fi(path);
        if (fi.isDir())
            model->removeDir(path, false, true, result);
        else
            result = removeFile(path, false);
        if (result == FileOpResult::SUCCESS)
            ++successCount;
    }

    if (paths.count() == 1) {
        if (result == FileOpResult::SUCCESS)
            mw->showMessageSuccess(tr("File removed"));
        else
            outputError(result);
    } else if (paths.count() > 1) {
        mw->showMessageSuccess(tr("Removed: ") + QString::number(successCount) + tr(" files"));
    }
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

    FileOpResult result{};
    unsigned     successCount = 0;
    for (auto const &path : paths) {
        result = removeFile(path, true);
        if (result == FileOpResult::SUCCESS) {
            ++successCount;
        } else if (
            mw->showConfirmation(
                tr("Error"),
                tr("There was an error when attempting to move a file to the trash. "
                   "Would you like to try permanently deleting it instead?"),
                QMessageBox::No))
        {
            removeFile(path, false);
        }
    }

    if (paths.count() == 1) {
        if (result == FileOpResult::SUCCESS) {
            mw->showMessageSuccess(tr("Moved to trash"));
        } else {
            outputError(result);

        }
    } else if (paths.count() > 1) {
        mw->showMessageSuccess(tr("Moved to trash: ") + QString::number(successCount) + tr(" files"));
    }
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
    mimeData->setData(u"x-special/gnome-copied-files"_s, gnomeFormat);
    mimeData->setData(u"application/x-kde-cutselection"_s, "0");

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

        QString tmpPath = destPath + u'_' +
                          QString::fromLatin1(QCryptographicHash::hash(destPath.toUtf8(), QCryptographicHash::Md5).toHex());
        QFileInfo fi(destPath);
        QString   ext     = fi.suffix();
        int       quality = 95;

        if (ext.compare(u"png"_sv, Qt::CaseInsensitive) == 0)
            quality = 30;
        else if (ext.compare(u"jpg"_sv, Qt::CaseInsensitive) == 0 || ext.compare(u"jpeg"_sv, Qt::CaseInsensitive) == 0)
            quality = settings->JPEGSaveQuality();

        bool backupExists   = false;
        bool originalExists = false;

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
        bool success = image.save(destPath, ext.toUtf8().data(), quality);

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

//-----------------------------------------------------------------------------------

#ifdef Q_OS_WINDOWS
# define GetPIDLFolder(pida)  (reinterpret_cast<LPCITEMIDLIST>((reinterpret_cast<BYTE const *>(pida)) + (pida)->aoffset[0]))
# define GetPIDLItem(pida, i) (reinterpret_cast<LPCITEMIDLIST>((reinterpret_cast<BYTE const *>(pida)) + (pida)->aoffset[(i) + 1]))

static void dumpComError(char16_t const *message, ::HRESULT res)
{
    qDebug() << u"COM error in" << message << u':'
             << QString::number(static_cast<ULONG>(res), 16).toUpper() << u':'
             << QString::fromStdString(std::error_code(res, std::system_category()).message());
}

ND static QStringList getFilePathsViaWin32Idlist(QMimeData const *mimeData)
{
    auto shIdList = mimeData->data(uR"(application/x-qt-windows-mime;value="Shell IDList Array")"_s);
    auto idList   = reinterpret_cast<::CIDA const *>(shIdList.data());

    LPWSTR  str = nullptr;
    HRESULT res = ::SHGetNameFromIDList(GetPIDLFolder(idList), SIGDN_DESKTOPABSOLUTEPARSING, &str);
    if (FAILED(res)) {
        dumpComError(u"SHGetNameFromIDList", res);
        return {};
    }
    QString rootDir = QString::fromWCharArray(str);
    rootDir.append(u'\\');
    ::CoTaskMemFree(str);

    QStringList ret;
    ret.reserve(idList->cidl);
    for (unsigned i = 0; i < idList->cidl; ++i) {
        str = nullptr;
        res = ::SHGetNameFromIDList(GetPIDLItem(idList, i), SIGDN_PARENTRELATIVE, &str);
        if (FAILED(res)) {
            dumpComError(u"SHGetNameFromIDList", res);
            continue;
        }
        QString path = rootDir;
        path.append(reinterpret_cast<QChar *>(str), static_cast<qsizetype>(wcslen(str)));
        ::CoTaskMemFree(str);
        ret.append(std::move(path));
    }
    return ret;
}

# undef GetPIDLFolder
# undef GetPIDLItem
#endif

//-----------------------------------------------------------------------------------

void Core::onDropIn(QMimeData const *mimeData, QObject const *source)
{
    // Ignore self.
    if (source == this)
        return;
    if (!mimeData->hasUrls())
        return;

    QList<QUrl> urlList = mimeData->urls();
    QStringList pathList;

    if (urlList.isEmpty()) {
        qWarning() << u"hasUrls() indicated that file urls should exist, but urls() returned an empty list.";
#ifdef Q_OS_WINDOWS
        pathList = getFilePathsViaWin32Idlist(mimeData);
        if (pathList.isEmpty())
            qWarning() << u"The backup approach failed to determine any file path.";
#endif
    } else {
        // Extract the local paths of the files.
        for (auto const &i : urlList)
            pathList.append(i.toLocalFile());
    }

    // Try to open first file in the list.
    if (pathList.isEmpty())
        qWarning() << u"No files included with drag-and-drop event: ignoring.";
    else
        loadPath(pathList.first());
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
        for (auto const &path : paths) {
            QUrl tmp = QUrl::fromLocalFile(path);
            urlList.append(tmp);
        }
        mimeData->setUrls(urlList);
    }
    auto *drag = new QDrag(this);
    // auto thumb = Thumbnailer::getThumbnail(paths.last(), 100);
    drag->setMimeData(mimeData);
    // mDrag->setPixmap(*thumb->pixmap().get());
    drag->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, Qt::CopyAction);
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
            path = settings->tmpDir() + u"image.png";
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

void Core::onFileRemoved(QString const &filePath, qsizetype index)
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

void Core::onFileRenamed(QString const &fromPath, qsizetype /*indexFrom*/, QString const &/*toPath*/, qsizetype indexTo)
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
    QString fm = ScriptManager::runCommand(u"xdg-mime query default inode/directory"_s);
    if (fm.contains(u"dolphin"_sv))
        ScriptManager::runCommandDetached(u"dolphin --select " + selectedPath());
    else if (fm.contains(u"nautilus"_sv))
        ScriptManager::runCommandDetached(u"nautilus --select " + selectedPath());
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(model->directoryPath()));
#elif defined Q_OS_WIN32
    QStringList args = {u"/select,"_s, QDir::toNativeSeparators(selectedPath())};
    QProcess::startDetached(u"explorer"_s, args);
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
    } else if (!dstDir.mkpath(u"."_s)) {
        mw->showError(tr("Could not create directory ") + dstDir.absolutePath());
        qDebug() << u"Could not create directory" << dstDir.absolutePath();
        return;
    }
    // copy all contents
    // TODO: skip symlinks? test
    QStringList entryList = srcDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
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
    } else if (!dstDir.mkpath(u"."_s)) {
        mw->showError(tr("Could not create directory ") + dstDir.absolutePath());
        qDebug() << u"Could not create directory" << dstDir.absolutePath();
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
    if (save && !mw->showConfirmation(actionName, tr("Perform action \"") + actionName + u"\"? \n\n" +
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
    edit_template(mw->currentViewMode() == ViewMode::FOLDERVIEW, tr("Flip horizontal"), {ImageLib::flippedH});
}

void Core::flipV()
{
    edit_template(mw->currentViewMode() == ViewMode::FOLDERVIEW, tr("Flip vertical"), {ImageLib::flippedV});
}

void Core::rotateByDegrees(int degrees)
{
    edit_template(mw->currentViewMode() == ViewMode::FOLDERVIEW, tr("Rotate"), {ImageLib::rotated}, degrees);
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
QString Core::selectedPath() const
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
        mw->showMessage(u"Set wallpaper: file not supported"_s);
        return;
    }

#ifdef Q_OS_WIN32
    // Set fit mode (registry).
    HKEY    hKey;
    LSTATUS status = ::RegOpenKeyExW(HKEY_CURRENT_USER, LR"(Control Panel\Desktop)", 0, KEY_WRITE, &hKey);
    if (status == ERROR_SUCCESS && hKey != nullptr) {
        static constexpr wchar_t data[]  = L"10";
        status = ::RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, reinterpret_cast<BYTE const *>(data), sizeof data);
        ::RegCloseKey(hKey);
    }
    // Set wallpaper path.
    ::SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0,
                            const_cast<void *>(static_cast<void const *>(selectedPath().toStdWString().c_str())),
                            SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#elif defined Q_OS_LINUX || (defined Q_OS_UNIX && !defined Q_OS_MACOS)
    auto session = qgetenv("DESKTOP_SESSION").toLower();
    if (session.contains("plasma"))
        ScriptManager::runCommand(u"plasma-apply-wallpaperimage \"" + selectedPath() + u"\"");
    else if (session.contains("gnome"))
        ScriptManager::runCommand(u"gsettings set org.gnome.desktop.background picture-uri \"" + selectedPath() + u"\"");
    else
        mw->showMessage(u"Action is not supported in your desktop session (\"" + QString::fromUtf8(session) + u'\"'), 3000);
#else
    mw->showMessage(u"Action is not supported on your OS"_sv, 3000);
#endif
}

void Core::print()
{
    if (model->isEmpty())
        return;
    auto p   = PrintDialog(mw.get());
    auto img = model->getImage(selectedPath());
    if (!img) {
        mw->showError(tr("Could not open image"));
        return;
    }
    if (img->type() != DocumentType::STATIC) {
        mw->showError(tr("Can only print static images"));
        return;
    }
    QString pdfPath = model->directoryPath() + u'/' + img->baseName() + u".pdf";
    p.setImage(img->getImage());
    p.setOutputPath(pdfPath);
    p.exec();
}

void Core::scalingRequest(QSize size, ScalingFilter filter)
{
    // filter out an unnecessary scale request at statup
    if (mw->isVisible() && state.hasActiveImage) {
        if (QSharedPointer<Image> forScale = model->getImage(state.currentFilePath))
            model->scaler()->requestScaled(ScalerRequest(forScale, size, state.currentFilePath, filter));
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
    if (path.startsWith(u"file://"_sv, Qt::CaseInsensitive))
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
        qDebug() << u"Could not open path:" << path;
        return false;
    }

    if (!state.delayModel && !setDirectory(state.directoryPath))
        return false;

    // load file / folderview
    if (fileInfo.isFile()) {
        auto index = model->indexOfFile(fileInfo.absoluteFilePath());
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

bool Core::loadFileIndex(qsizetype index, bool async, bool preload)
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
        auto dm = std::make_unique<DirectoryManager>();
        if (!dm->setDirectory(parentDir.absoluteFilePath()))
            return;

        QString next = dm->nextOfDir(model->directoryPath());
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
        auto dm = std::make_unique<DirectoryManager>();
        dm->setDirectory(parentDir.absoluteFilePath());

        QString prev = dm->prevOfDir(model->directoryPath());
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
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW || (model->isEmpty() && folderEndAction != FolderEndAction::GOTO_ADJACENT))
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
    if (mw->currentViewMode() == ViewMode::FOLDERVIEW || (model->isEmpty() && folderEndAction != FolderEndAction::GOTO_ADJACENT))
        return;
    stopSlideshow();
    if (shuffle) {
        loadFileIndex(randomizer.prev(), true, false);
        return;
    }

    auto newIndex = model->indexOfFile(state.currentFilePath) - 1;
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
        auto newIndex = model->indexOfFile(state.currentFilePath) + 1;
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

void Core::onLoadFailed(QString const &path)
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
