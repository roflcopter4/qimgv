#include "MainWindow.h"
#include <stdexcept>

// TODO: nuke this and rewrite

MW::MW(QWidget *parent)
    : FloatingWidgetContainer(parent),
      layout(new QHBoxLayout(this)),
      windowGeometryChangeTimer(new QTimer(this))
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setMinimumSize(10, 10);

    // do not steal focus when clicked
    // this is just a container. accept key events only
    // via passthrough from child widgets
    setFocusPolicy(Qt::NoFocus);
    setLayout(layout);
    setWindowTitle(QCoreApplication::applicationName() + u' ' + QCoreApplication::applicationVersion());
    setMouseTracking(true);
    setAcceptDrops(true);
    setAccessibleName(QS("mainwindow"));
    windowGeometryChangeTimer->setSingleShot(true);
    windowGeometryChangeTimer->setInterval(30);
    setupUi();

    connect(settings, &Settings::settingsChanged, this, &MW::readSettings);
    connect(windowGeometryChangeTimer, &QTimer::timeout, this, &MW::onWindowGeometryChanged);
    connect(this, &MW::fullscreenStateChanged, this, &MW::adaptToWindowState);

    readSettings();
    currentDisplay = settings->lastDisplay();
    maximized      = settings->maximizedWindow();
    restoreWindowGeometry();
}

MW::~MW()
{
    util::DeleteAndNullify(infoBarFullscreen);
    util::DeleteAndNullify(imageInfoOverlay);
    util::DeleteAndNullify(floatingMessage);
}

/*                                                             |--[ImageViewer]
 *                        |--[DocumentWidget]--[ViewerWidget]--|
 * [MW]--[CentralWidget]--|                                    |--[VideoPlayer]
 *                        |--[FolderView]
 *
 *  (not counting floating widgets)
 *  ViewerWidget exists for input handling reasons (correct overlay hover handling)
 */
void MW::setupUi()
{
    viewerWidget    = new ViewerWidget(this);
    infoBarWindowed = new InfoBarProxy(this);
    docWidget       = new DocumentWidget(viewerWidget, infoBarWindowed, this);
    folderView      = new FolderViewProxy(this);
    
    connect(folderView, &FolderViewProxy::sortingSelected,    this, &MW::sortingSelected);
    connect(folderView, &FolderViewProxy::directorySelected,  this, &MW::opened);
    connect(folderView, &FolderViewProxy::copyUrlsRequested,  this, &MW::copyUrlsRequested);
    connect(folderView, &FolderViewProxy::moveUrlsRequested,  this, &MW::moveUrlsRequested);
    connect(folderView, &FolderViewProxy::showFoldersChanged, this, &MW::showFoldersChanged);

    controlsOverlay   = new ControlsOverlay(docWidget);
    infoBarFullscreen = new FullscreenInfoOverlayProxy(viewerWidget);
    imageInfoOverlay  = new ImageInfoOverlayProxy(viewerWidget);
    floatingMessage   = new FloatingMessageProxy(viewerWidget); // todo: use additional one for folderview?
    sidePanel         = new SidePanel(this);

    centralWidget = new CentralWidget(docWidget, folderView, this);
    layout->addWidget(centralWidget);
    layout->addWidget(sidePanel);

    connect(viewerWidget, &ViewerWidget::scalingRequested,   this, &MW::scalingRequested);
    connect(viewerWidget, &ViewerWidget::draggedOut,         this, qOverload<>(&MW::draggedOut));
    connect(viewerWidget, &ViewerWidget::playbackFinished,   this, &MW::playbackFinished);
    connect(viewerWidget, &ViewerWidget::showScriptSettings, this, &MW::showScriptSettings);

    connect(this, &MW::zoomIn,                 viewerWidget, &ViewerWidget::zoomIn);
    connect(this, &MW::zoomOut,                viewerWidget, &ViewerWidget::zoomOut);
    connect(this, &MW::zoomInCursor,           viewerWidget, &ViewerWidget::zoomInCursor);
    connect(this, &MW::zoomOutCursor,          viewerWidget, &ViewerWidget::zoomOutCursor);
    connect(this, &MW::scrollUp,               viewerWidget, &ViewerWidget::scrollUp);
    connect(this, &MW::scrollDown,             viewerWidget, &ViewerWidget::scrollDown);
    connect(this, &MW::scrollLeft,             viewerWidget, &ViewerWidget::scrollLeft);
    connect(this, &MW::scrollRight,            viewerWidget, &ViewerWidget::scrollRight);
    connect(this, &MW::pauseVideo,             viewerWidget, &ViewerWidget::pauseResumePlayback);
    connect(this, &MW::stopPlayback,           viewerWidget, &ViewerWidget::stopPlayback);
    connect(this, &MW::seekVideoForward,       viewerWidget, &ViewerWidget::seekForward);
    connect(this, &MW::seekVideoBackward,      viewerWidget, &ViewerWidget::seekBackward);
    connect(this, &MW::frameStep,              viewerWidget, &ViewerWidget::frameStep);
    connect(this, &MW::frameStepBack,          viewerWidget, &ViewerWidget::frameStepBack);
    connect(this, &MW::toggleMute,             viewerWidget, &ViewerWidget::toggleMute);
    connect(this, &MW::volumeUp,               viewerWidget, &ViewerWidget::volumeUp);
    connect(this, &MW::volumeDown,             viewerWidget, &ViewerWidget::volumeDown);
    connect(this, &MW::toggleTransparencyGrid, viewerWidget, &ViewerWidget::toggleTransparencyGrid);
    connect(this, &MW::setLoopPlayback,        viewerWidget, &ViewerWidget::setLoopPlayback);
}

void MW::setupFullUi()
{
    setupCropPanel();
    docWidget->allowPanelInit();
    docWidget->setupMainPanel();
    infoBarWindowed->init();
    infoBarFullscreen->init();
}

void MW::setupCropPanel()
{
    if (cropPanel)
        return;
    cropOverlay = new CropOverlay(viewerWidget);
    cropPanel   = new CropPanel(cropOverlay, this);

    connect(cropPanel, &CropPanel::cancel,      this, &MW::hideCropPanel);
    connect(cropPanel, &CropPanel::crop,        this, &MW::hideCropPanel);
    connect(cropPanel, &CropPanel::crop,        this, &MW::cropRequested);
    connect(cropPanel, &CropPanel::cropAndSave, this, &MW::hideCropPanel);
    connect(cropPanel, &CropPanel::cropAndSave, this, &MW::cropAndSaveRequested);
}

void MW::setupCopyOverlay()
{
    if (copyOverlay)
        throw std::logic_error("Overlay already exists.");
    copyOverlay = new CopyOverlay(viewerWidget);
    connect(copyOverlay, &CopyOverlay::copyRequested, this, &MW::copyRequested);
    connect(copyOverlay, &CopyOverlay::moveRequested, this, &MW::moveRequested);
}

void MW::setupSaveOverlay()
{
    if (saveOverlay)
        throw std::logic_error("Overlay already exists.");
    saveOverlay = new SaveConfirmOverlay(viewerWidget);
    connect(saveOverlay, &SaveConfirmOverlay::saveClicked,    this, &MW::saveRequested);
    connect(saveOverlay, &SaveConfirmOverlay::saveAsClicked,  this, &MW::saveAsClicked);
    connect(saveOverlay, &SaveConfirmOverlay::discardClicked, this, &MW::discardEditsRequested);
}

void MW::setupRenameOverlay()
{
    if (renameOverlay)
        throw std::logic_error("Overlay already exists.");
    renameOverlay = new RenameOverlay(this);
    renameOverlay->setName(info.fileName);
    connect(renameOverlay, &RenameOverlay::renameRequested, this, &MW::renameRequested);
}

void MW::toggleFolderView()
{
    hideCropPanel();
    if (copyOverlay)
        copyOverlay->hide();
    if (renameOverlay)
        renameOverlay->hide();
    docWidget->hideFloatingPanel();
    imageInfoOverlay->hide();
    centralWidget->toggleViewMode();
    onInfoUpdated();
}

void MW::enableFolderView()
{
    hideCropPanel();
    if (copyOverlay)
        copyOverlay->hide();
    if (renameOverlay)
        renameOverlay->hide();
    docWidget->hideFloatingPanel();
    imageInfoOverlay->hide();
    centralWidget->showFolderView();
    onInfoUpdated();
}

void MW::enableDocumentView()
{
    centralWidget->showDocumentView();
    onInfoUpdated();
}

ViewMode MW::currentViewMode() const
{
    return centralWidget->currentViewMode();
}

void MW::fitWindow()
{
    if (viewerWidget->interactionEnabled())
        emit viewerWidget->fitWindow();
    else
        showMessage(QS("Zoom temporary disabled"));
}

void MW::fitWidth()
{
    if (viewerWidget->interactionEnabled())
        emit viewerWidget->fitWidth();
    else
        showMessage(QS("Zoom temporary disabled"));
}

void MW::fitOriginal()
{
    if (viewerWidget->interactionEnabled())
        emit viewerWidget->fitOriginal();
    else
        showMessage(QS("Zoom temporary disabled"));
}

// switch between 1:1 and Fit All
// TODO: move to viewerWidget?
void MW::switchFitMode()
{
    if (viewerWidget->fitMode() == ImageFitMode::WINDOW)
        viewerWidget->setFitMode(ImageFitMode::ORIGINAL);
    else
        viewerWidget->setFitMode(ImageFitMode::WINDOW);
}

void MW::closeImage()
{
    info.fileName = QString();
    info.filePath = QString();
    viewerWidget->closeImage();
}

// todo: fix flicker somehow
// ideally it should change img & resize in one go
void MW::preShowResize(QSize sz)
{
    auto screens = qApp->screens();

    if (windowState() != Qt::WindowNoState || !screens.count() || screens.count() <= currentDisplay)
        return;

    int   decorationSize = frameGeometry().height() - height();
    qreal maxSzMulti     = settings->autoResizeLimit() / 100.0;
    QRect availableGeom  = screens.at(currentDisplay)->availableGeometry();
    QSize maxSz          = availableGeom.size() * maxSzMulti;
    maxSz.setHeight(maxSz.height() - decorationSize);

    if (!sz.isEmpty()) {
        if (sz.width() > maxSz.width() || sz.height() > maxSz.height())
            sz.scale(maxSz, Qt::KeepAspectRatio);
    } else {
        sz = maxSz;
    }

    QRect newGeom(0, 0, sz.width(), sz.height());
    newGeom.moveCenter(availableGeom.center());
    newGeom.translate(0, decorationSize / 2);

    if (isVisible())
        setGeometry(newGeom);
    else // setGeometry won't work on hidden windows, so we just save for it to be restored later
        settings->setWindowGeometry(newGeom);
}

void MW::showImage(std::unique_ptr<QPixmap> pixmap)
{
    if (settings->autoResizeWindow())
        preShowResize(pixmap->size());
    viewerWidget->showImage(std::move(pixmap));
    updateCropPanelData();
}

void MW::showAnimation(QSharedPointer<QMovie> const &movie)
{
    if (settings->autoResizeWindow())
        preShowResize(movie->frameRect().size());
    viewerWidget->showAnimation(movie);
    updateCropPanelData();
}

void MW::showVideo(QString const &file)
{
    if (settings->autoResizeWindow())
        preShowResize(QSize()); // tmp. find a way to get this though mpv BEFORE playback
    viewerWidget->showVideo(file);
}

void MW::showContextMenu()
{
    viewerWidget->showContextMenu();
}

void MW::onSortingChanged(SortingMode mode)
{
    folderView->onSortingChanged(mode);
    if (centralWidget->currentViewMode() == ViewMode::DOCUMENT) {
        switch (mode) {
        case SortingMode::NAME:      showMessage(QS("Sorting: By Name"));              break;
        case SortingMode::NAME_DESC: showMessage(QS("Sorting: By Name (desc.)"));      break;
        case SortingMode::TIME:      showMessage(QS("Sorting: By Time"));              break;
        case SortingMode::TIME_DESC: showMessage(QS("Sorting: By Time (desc.)"));      break;
        case SortingMode::SIZE:      showMessage(QS("Sorting: By File Size"));         break;
        case SortingMode::SIZE_DESC: showMessage(QS("Sorting: By File Size (desc.)")); break;
        }
    }
}

void MW::setDirectoryPath(QString const &path)
{
    // closeImage();
    info.directoryPath = path;
    info.directoryName = path.split(u'/').last();
    folderView->setDirectoryPath(path);
    onInfoUpdated();
}

void MW::toggleLockZoom()
{
    emit viewerWidget->toggleLockZoom();
    if (viewerWidget->lockZoomEnabled())
        showMessage(QS("Zoom lock: ON"));
    else
        showMessage(QS("Zoom lock: OFF"));
    onInfoUpdated();
}

void MW::toggleLockView()
{
    emit viewerWidget->toggleLockView();
    if (viewerWidget->lockViewEnabled())
        showMessage(QS("View lock: ON"));
    else
        showMessage(QS("View lock: OFF"));
    onInfoUpdated();
}

void MW::toggleFullscreenInfoBar()
{
    if (!isFullScreen())
        return;
    showInfoBarFullscreen = !showInfoBarFullscreen;
    if (showInfoBarFullscreen)
        infoBarFullscreen->showWhenReady();
    else
        infoBarFullscreen->hide();
}

void MW::toggleImageInfoOverlay()
{
    if (centralWidget->currentViewMode() == ViewMode::FOLDERVIEW)
        return;
    if (imageInfoOverlay->isHidden())
        imageInfoOverlay->show();
    else
        imageInfoOverlay->hide();
}

void MW::toggleRenameOverlay(QString const &currentName)
{
    if (!renameOverlay)
        setupRenameOverlay();
    if (renameOverlay->isHidden()) {
        renameOverlay->setBackdropEnabled((centralWidget->currentViewMode() == ViewMode::FOLDERVIEW));
        renameOverlay->setName(currentName);
        renameOverlay->show();
    } else {
        renameOverlay->hide();
    }
}

void MW::toggleScalingFilter()
{
    ScalingFilter configuredFilter = settings->scalingFilter();
    if (viewerWidget->scalingFilter() == configuredFilter)
        setFilterNearest();
    else
        setFilter(configuredFilter);
}

void MW::setFilterNearest()
{
    showMessage(QS("Filter: nearest"), 600);
    emit viewerWidget->setFilterNearest();
}

void MW::setFilterBilinear()
{
    showMessage(QS("Filter: bilinear"), 600);
    emit viewerWidget->setFilterBilinear();
}

void MW::setFilter(ScalingFilter filter)
{
    QString filterName;
    switch (filter) {
    case ScalingFilter::NEAREST:             filterName = QS("nearest");            break;
    case ScalingFilter::BILINEAR:            filterName = QS("bilinear");           break;
    case ScalingFilter::CV_BILINEAR_SHARPEN: filterName = QS("bilinear + sharpen"); break;
    case ScalingFilter::CV_CUBIC:            filterName = QS("bicubic");            break;
    case ScalingFilter::CV_CUBIC_SHARPEN:    filterName = QS("bicubic + sharpen");  break;
    default:
        filterName = QSV("configured ") + QString::number(static_cast<int>(filter));
        break;
    }
    showMessage(QSV("Filter ") + filterName, 600);
    emit viewerWidget->setScalingFilter(filter);
}

bool MW::isCropPanelActive() const
{
    return (activeSidePanel == ActiveSidePanel::CROP);
}

void MW::onScalingFinished(std::unique_ptr<QPixmap> scaled)
{
    viewerWidget->onScalingFinished(std::move(scaled));
}

void MW::saveWindowGeometry()
{
    if (windowState() == Qt::WindowNoState)
        settings->setWindowGeometry(geometry());
    settings->setMaximizedWindow(maximized);
}

// does not apply fullscreen; window size / maximized state only
void MW::restoreWindowGeometry()
{
    setGeometry(settings->windowGeometry());
    if (settings->maximizedWindow())
        setWindowState(Qt::WindowMaximized);
    updateCurrentDisplay();
}

void MW::updateCurrentDisplay()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    currentDisplay = desktopWidget.screenNumber(this);
#else
    auto screens   = qApp->screens();
    currentDisplay = screens.indexOf(window()->screen());
#endif
}

void MW::onWindowGeometryChanged()
{
    saveWindowGeometry();
    updateCurrentDisplay();
}

void MW::saveCurrentDisplay()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    settings->setLastDisplay(desktopWidget.screenNumber(this));
#else
    settings->setLastDisplay(qApp->screens().indexOf(window()->screen()));
#endif
}

// #############################################################
// ######################### EVENTS ############################
// #############################################################

void MW::mouseMoveEvent(QMouseEvent *event)
{
    event->ignore();
}

bool MW::event(QEvent *event)
{
    // Only save maximized state if we are already visible.
    // This filters out events while the window is still being set up.
    if (event->type() == QEvent::WindowStateChange && isVisible() && !isFullScreen())
        maximized = isMaximized();
    else if (event->type() == QEvent::Move || event->type() == QEvent::Resize)
        windowGeometryChangeTimer->start();
    return QWidget::event(event);
}

// hook up to actionManager
void MW::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    actionManager->processEvent(event);
}

void MW::wheelEvent(QWheelEvent *event)
{
    event->accept();
    actionManager->processEvent(event);
}

void MW::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    actionManager->processEvent(event);
}

void MW::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    actionManager->processEvent(event);
}

void MW::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
    auto fakePressEvent = QMouseEvent{
        QEvent::MouseButtonPress,
        event->position(), event->globalPosition(),
        event->button(), event->buttons(), event->modifiers()
    };
    actionManager->processEvent(&fakePressEvent);
    actionManager->processEvent(event);
}

void MW::close()
{
    saveWindowGeometry();
    saveCurrentDisplay();
    // try to close window sooner
    // since qt6.3 QWidget::close() no longer works on hidden windows (bug?)
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    hide();
#endif
    if (copyOverlay)
        copyOverlay->saveSettings();
    QWidget::close();
}

void MW::closeEvent(QCloseEvent *event)
{
    // catch the close event when user presses X on the window itself
    event->accept();
    actionManager->invokeAction(QS("exit"));
}

void MW::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MW::dropEvent(QDropEvent *event)
{
    emit droppedIn(event->mimeData(), event->source());
}

void MW::resizeEvent(QResizeEvent *event)
{
    if (activeSidePanel == ActiveSidePanel::CROP) {
        cropOverlay->setImageScale(viewerWidget->currentScale());
        cropOverlay->setImageDrawRect(viewerWidget->imageRect());
    }
    FloatingWidgetContainer::resizeEvent(event);
}

void MW::showDefault()
{
    if (!isVisible()) {
        if (settings->fullscreenMode())
            showFullScreen();
        else
            showWindowed();
    }
}

void MW::showSaveDialog(QString const &filePath)
{
    QString newFilePath = getSaveFileName(filePath);
    if (!newFilePath.isEmpty())
        emit saveAsRequested(newFilePath);
}

QString MW::getSaveFileName(QString const &filePath)
{
    docWidget->hideFloatingPanel();
    QStringList filters;
    // Generate filter for writable images.
    // todo: some may need to be blacklisted
    QList<QByteArray> writerFormats = QImageWriter::supportedImageFormats();
    if (writerFormats.contains(QByteArrayView("jpg")))
        filters.append(QS("JPEG (*.jpg *.jpeg *jpe *jfif)"));
    if (writerFormats.contains(QByteArrayView("png")))
        filters.append(QS("PNG (*.png)"));
    if (writerFormats.contains(QByteArrayView("webp")))
        filters.append(QS("WebP (*.webp)"));
    // May not work...
    if (writerFormats.contains(QByteArrayView("jp2")))
        filters.append(QS("JPEG 2000 (*.jp2 *.j2k *.jpf *.jpx *.jpm *.jpgx)"));
    if (writerFormats.contains(QByteArrayView("jxl")))
        filters.append(QS("JPEG-XL (*.jxl)"));
    if (writerFormats.contains(QByteArrayView("avif")))
        filters.append(QS("AVIF (*.avif *.avifs)"));
    if (writerFormats.contains(QByteArrayView("tif")))
        filters.append(QS("TIFF (*.tif *.tiff)"));
    if (writerFormats.contains(QByteArrayView("bmp")))
        filters.append(QS("BMP (*.bmp)"));
#ifdef Q_OS_WIN32
    if (writerFormats.contains(QByteArrayView("ico")))
        filters.append(QS("Icon Files (*.ico)"));
#endif
    if (writerFormats.contains(QByteArrayView("ppm")))
        filters.append(QS("PPM (*.ppm)"));
    if (writerFormats.contains(QByteArrayView("xbm")))
        filters.append(QS("XBM (*.xbm)"));
    if (writerFormats.contains(QByteArrayView("xpm")))
        filters.append(QS("XPM (*.xpm)"));
    if (writerFormats.contains(QByteArrayView("dds")))
        filters.append(QS("DDS (*.dds)"));
    if (writerFormats.contains(QByteArrayView("wbmp")))
        filters.append(QS("WBMP (*.wbmp)"));
    // Add everything else from imagewriter.
    for (auto const &fmt : writerFormats) {
        auto qsFmt = QString::fromUtf8(fmt);
        if (filters.filter(qsFmt).isEmpty())
            filters.append(qsFmt.toUpper() + QSV(" (*.") + qsFmt + u')');
    }
    QString filterString = filters.join(QSV(";; "));

    // Find matching filter for the current image.
    auto selectedFilter = QS("JPEG (*.jpg *.jpeg *jpe *jfif)");
    auto fi = QFileInfo(filePath);

    for (auto const &filter : filters) {
        if (filter.contains(fi.suffix().toLower())) {
            selectedFilter = filter;
            break;
        }
    }
    QString newFilePath = QFileDialog::getSaveFileName(this, tr("Save File as..."), filePath, filterString, &selectedFilter);
    return newFilePath;
}

void MW::showOpenDialog(QString const &path)
{
    docWidget->hideFloatingPanel();

    QFileDialog dialog(this);
    QStringList imageFilter;
    imageFilter.append(settings->supportedFormatsFilter());
    imageFilter.append(QS("All Files (*)"));
    dialog.setDirectory(path);
    dialog.setNameFilters(imageFilter);
    dialog.setWindowTitle(QS("Open image"));
    dialog.setWindowModality(Qt::ApplicationModal);
    connect(&dialog, &QFileDialog::fileSelected, this, &MW::opened);
    dialog.exec();
}

void MW::showResizeDialog(QSize initialSize)
{
    ResizeDialog dialog(initialSize, this);
    connect(&dialog, &ResizeDialog::sizeSelected, this, &MW::resizeRequested);
    dialog.exec();
}

DialogResult MW::fileReplaceDialog(QString const &source, QString const &dest, FileReplaceMode mode, bool multiple)
{
    FileReplaceDialog dialog(this);
    dialog.setModal(true);
    dialog.setSource(source);
    dialog.setDestination(dest);
    dialog.setMode(mode);
    dialog.setMulti(multiple);
    dialog.exec();
    return dialog.getResult();
}

void MW::showSettings()
{
    docWidget->hideFloatingPanel();
    SettingsDialog settingsDialog(this);
    settingsDialog.exec();
}

void MW::showScriptSettings()
{
    docWidget->hideFloatingPanel();
    SettingsDialog settingsDialog(this);
    settingsDialog.switchToPage(4);
    settingsDialog.exec();
}

void MW::triggerFullScreen()
{
    if (!isFullScreen())
        showFullScreen();
    else
        showWindowed();
}

void MW::showFullScreen()
{
    // do not save immediately on application start
    if (!isHidden())
        saveWindowGeometry();
    auto screens = qApp->screens();

    // todo: why check the screen again?
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qsizetype trueCurrentDisplay = desktopWidget.screenNumber(this);
#else
    qsizetype trueCurrentDisplay = screens.indexOf(window()->screen());
#endif

    // move to target screen
    if (screens.count() > currentDisplay && currentDisplay != trueCurrentDisplay) {
        auto const &geom = screens[currentDisplay]->geometry();
        move(geom.x(), geom.y());
    }
    QWidget::showFullScreen();

    //// try to repaint sooner
    //qApp->processEvents();
    emit fullscreenStateChanged(true);
}

void MW::showWindowed()
{
    if (isFullScreen())
        QWidget::showNormal();
    restoreWindowGeometry();
    QWidget::show();
    //// try to repaint sooner
    //qApp->processEvents();
    emit fullscreenStateChanged(false);
}

void MW::updateCropPanelData()
{
    if (cropPanel && activeSidePanel == ActiveSidePanel::CROP) {
        cropPanel->setImageRealSize(viewerWidget->sourceSize());
        cropOverlay->setImageDrawRect(viewerWidget->imageRect());
        cropOverlay->setImageScale(viewerWidget->currentScale());
        cropOverlay->setImageRealSize(viewerWidget->sourceSize());
    }
}

void MW::showSaveOverlay()
{
    if (!settings->showSaveOverlay())
        return;
    if (!saveOverlay)
        setupSaveOverlay();
    saveOverlay->show();
}

void MW::hideSaveOverlay()
{
    if (!saveOverlay)
        return;
    saveOverlay->hide();
}

void MW::showChangelogWindow()
{
    changelogWindow->show();
}

void MW::showChangelogWindow(QString const &text)
{
    changelogWindow->setText(text);
    changelogWindow->show();
}

void MW::triggerCropPanel()
{
    if (activeSidePanel != ActiveSidePanel::CROP)
        showCropPanel();
    else
        hideCropPanel();
}

void MW::showCropPanel()
{
    if (centralWidget->currentViewMode() == ViewMode::FOLDERVIEW)
        return;

    if (activeSidePanel != ActiveSidePanel::CROP) {
        docWidget->hideFloatingPanel();
        sidePanel->setWidget(cropPanel);
        sidePanel->show();
        cropOverlay->show();
        activeSidePanel = ActiveSidePanel::CROP;
        // reset & lock zoom so CropOverlay won't go crazy
        emit viewerWidget->fitWindow();
        setInteractionEnabled(false);
        // feed the panel current image info
        updateCropPanelData();
    }
}

void MW::setInteractionEnabled(bool mode)
{
    docWidget->setInteractionEnabled(mode);
    viewerWidget->setInteractionEnabled(mode);
}

void MW::hideCropPanel()
{
    sidePanel->hide();
    if (activeSidePanel == ActiveSidePanel::CROP) {
        cropOverlay->hide();
        setInteractionEnabled(true);
    }
    activeSidePanel = ActiveSidePanel::NONE;
}

void MW::triggerCopyOverlay()
{
    if (!viewerWidget->isDisplaying())
        return;
    if (!copyOverlay)
        setupCopyOverlay();

    if (centralWidget->currentViewMode() == ViewMode::FOLDERVIEW)
        return;
    if (copyOverlay->operationMode() == CopyOverlayMode::COPY) {
        copyOverlay->isHidden() ? copyOverlay->show() : copyOverlay->hide();
    } else {
        copyOverlay->setDialogMode(CopyOverlayMode::COPY);
        copyOverlay->show();
    }
}

void MW::triggerMoveOverlay()
{
    if (!viewerWidget->isDisplaying())
        return;
    if (!copyOverlay)
        setupCopyOverlay();

    if (centralWidget->currentViewMode() == ViewMode::FOLDERVIEW)
        return;
    if (copyOverlay->operationMode() == CopyOverlayMode::MOVE) {
        copyOverlay->isHidden() ? copyOverlay->show() : copyOverlay->hide();
    } else {
        copyOverlay->setDialogMode(CopyOverlayMode::MOVE);
        copyOverlay->show();
    }
}

// quit fullscreen or exit the program
void MW::closeFullScreenOrExit()
{
    if (isFullScreen())
        showWindowed();
    else
        actionManager->invokeAction(QS("exit"));
}

// todo: this is crap, use shared state object
void MW::setCurrentInfo(qsizetype      fileIndex,
                        qsizetype      fileCount,
                        QString const &filePath,
                        QString const &fileName,
                        QSize          imageSize,
                        qint64         fileSize,
                        bool           slideshow,
                        bool           shuffle,
                        bool           edited)
{
    info.index     = fileIndex;
    info.fileCount = fileCount;
    info.fileName  = fileName;
    info.filePath  = filePath;
    info.imageSize = imageSize;
    info.fileSize  = fileSize;
    info.slideshow = slideshow;
    info.shuffle   = shuffle;
    info.edited    = edited;
    onInfoUpdated();
}

// todo: nuke and rewrite
void MW::onInfoUpdated()
{
    QString posString;
    if (info.fileCount)
        posString = QSV("[ ") + QString::number(info.index + 1) + u"/" + QString::number(info.fileCount) + QSV(" ]");
    QString resString;
    if (info.imageSize.width())
        resString = QString::number(info.imageSize.width()) + QSV(" x ") + QString::number(info.imageSize.height());
    QString sizeString;
    if (info.fileSize)
        sizeString = locale().formattedDataSize(info.fileSize, 1);

    if (renameOverlay)
        renameOverlay->setName(info.fileName);

    QString windowTitle;
    if (centralWidget->currentViewMode() == ViewMode::FOLDERVIEW) {
        windowTitle = tr("Folder view");
        infoBarFullscreen->setInfo(QString(), tr("No file opened."), QString());
        infoBarWindowed->setInfo(QString(), tr("No file opened."), QString());
    } else if (info.fileName.isEmpty()) {
        windowTitle = qApp->applicationName();
        infoBarFullscreen->setInfo(QString(), tr("No file opened."), QString());
        infoBarWindowed->setInfo(QString(), tr("No file opened."), QString());
    } else {
        windowTitle = info.fileName;
        if (settings->windowTitleExtendedInfo()) {
            windowTitle.prepend(posString + QS("  "));
            if (!resString.isEmpty())
                windowTitle.append(QSV("  -  ") + resString);
            if (!sizeString.isEmpty())
                windowTitle.append(QSV("  -  ") + sizeString);
        }

        // toggleable states
        QString states;
        if (info.slideshow)
            states.append(QSV(" [slideshow]"));
        if (info.shuffle)
            states.append(QSV(" [shuffle]"));
        if (viewerWidget->lockZoomEnabled())
            states.append(QSV(" [zoom lock]"));
        if (viewerWidget->lockViewEnabled())
            states.append(QSV(" [view lock]"));

        if (!settings->infoBarWindowed() && !states.isEmpty())
            windowTitle.append(QSV(" -") + states);
        if (info.edited)
            windowTitle.prepend(QSV("* "));

        infoBarFullscreen->setInfo(posString,
                                   info.fileName + (info.edited ? QS("  *") : QString()),
                                   resString + QSV("  ") + sizeString);

        infoBarWindowed->setInfo(posString,
                                 info.fileName + (info.edited ? QS("  *") : QString()),
                                 resString + QSV("  ") + sizeString + u" " + states);
    }
    setWindowTitle(windowTitle);
}

// TODO!!! buffer this in mw
void MW::setExifInfo(QMap<QString, QString> const &xinfo)
{
    if (imageInfoOverlay)
        imageInfoOverlay->setExifInfo(xinfo);
}

FolderViewProxy *MW::getFolderView()
{
    return folderView;
}

ThumbnailStripProxy *MW::getThumbnailPanel()
{
    return docWidget->thumbPanel();
}

// todo: this is crap
void MW::showMessageDirectory(QString const &dirName)
{
    floatingMessage->showMessage(dirName, FloatingMessageIcon::DIRECTORY, 1700);
}

void MW::showMessageDirectoryEnd()
{
    floatingMessage->showMessage(QString(), FloatingWidgetPosition::RIGHT, FloatingMessageIcon::RIGHT_EDGE, 400);
}

void MW::showMessageDirectoryStart()
{
    floatingMessage->showMessage(QString(), FloatingWidgetPosition::LEFT, FloatingMessageIcon::LEFT_EDGE, 400);
}

void MW::showMessageFitWindow()
{
    floatingMessage->showMessage(tr("Fit Window"), FloatingMessageIcon::NONE, 350);
}

void MW::showMessageFitWidth()
{
    floatingMessage->showMessage(tr("Fit Width"), FloatingMessageIcon::NONE, 350);
}

void MW::showMessageFitOriginal()
{
    floatingMessage->showMessage(tr("Fit 1:1"), FloatingMessageIcon::NONE, 350);
}

void MW::showMessage(QString const &text)
{
    floatingMessage->showMessage(text, FloatingMessageIcon::NONE, 1500);
}

void MW::showMessage(QString const &text, int duration)
{
    floatingMessage->showMessage(text, FloatingMessageIcon::NONE, duration);
}

void MW::showMessageSuccess(QString const &text)
{
    floatingMessage->showMessage(text, FloatingMessageIcon::SUCCESS, 1500);
}

void MW::showWarning(QString const &text)
{
    floatingMessage->showMessage(text, FloatingMessageIcon::WARNING, 1500);
}

void MW::showError(QString const &text)
{
    floatingMessage->showMessage(text, FloatingMessageIcon::ERROR, 2800);
}

bool MW::showConfirmation(QString const &title, QString const &msg)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(msg);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setModal(true);
    return msgBox.exec() == QMessageBox::Yes;
}

void MW::readSettings()
{
    showInfoBarFullscreen = settings->infoBarFullscreen();
    showInfoBarWindowed   = settings->infoBarWindowed();
    adaptToWindowState();
}

// todo: remove/rename?
void MW::applyWindowedBackground()
{
#ifdef USE_KDE_BLUR
    if (settings->backgroundOpacity() == 1.0)
        KWindowEffects::enableBlurBehind(winId(), false);
    else
        KWindowEffects::enableBlurBehind(winId(), settings->blurBackground());
#endif
}

void MW::applyFullscreenBackground()
{
#ifdef USE_KDE_BLUR
    KWindowEffects::enableBlurBehind(winId(), false);
#endif
}

// changes ui elements according to fullscreen state
void MW::adaptToWindowState()
{
    docWidget->hideFloatingPanel();
    if (isFullScreen()) { //-------------------------------------- fullscreen ---
        applyFullscreenBackground();
        infoBarWindowed->hide();

        if (showInfoBarFullscreen)
            infoBarFullscreen->showWhenReady();
        else
            infoBarFullscreen->hide();

        auto pos = settings->panelPosition();
        if (!settings->panelEnabled() || pos == PanelPosition::BOTTOM || pos == PanelPosition::LEFT)
            controlsOverlay->show();
        else
            controlsOverlay->hide();
    } else { //------------------------------------------------------ window ---
        applyWindowedBackground();
        infoBarFullscreen->hide();

        if (showInfoBarWindowed)
            infoBarWindowed->show();
        else
            infoBarWindowed->hide();

        controlsOverlay->hide();
    }
    folderView->onFullscreenModeChanged(isFullScreen());
    docWidget->onFullscreenModeChanged(isFullScreen());
    viewerWidget->onFullscreenModeChanged(isFullScreen());
}

void MW::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    FloatingWidgetContainer::paintEvent(event);
}

void MW::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    docWidget->hideFloatingPanel(true);
}

// block native tab-switching so we can use it in shortcuts
// bool MW::focusNextPrevChild(bool) {
//    return false;
//}
