#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Preferences — ") + qApp->applicationName());

    ui->shortcutsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->aboutAppTextBrowser->viewport()->setAutoFillBackground(false);
    ui->versionLabel->setText(QApplication::applicationVersion());
    ui->qtVersionLabel->setText(QString::fromUtf8(qVersion()));
    ui->appIconLabel->setPixmap(QIcon(QS(":/res/icons/common/logo/app/22.png")).pixmap(22, 22));
    ui->qtIconLabel->setPixmap(QIcon(QS(":/res/icons/common/logo/3rdparty/qt22.png")).pixmap(22, 16));

    // fake combobox that acts as a menu button
    // less code than using pushbutton with menu
    // will be replaced with something custom later
    connect(
        ui->themeSelectorComboBox,
        qOverload<int>(&QComboBox::currentIndexChanged),
        [this](int index) {
            ui->themeSelectorComboBox->blockSignals(true);
            ui->themeSelectorComboBox->setCurrentIndex(index);
            ui->themeSelectorComboBox->blockSignals(false);
            switch (index) {
            case 0:
                setColorScheme(ThemeStore::colorScheme(ColorSchemes::BLACK));
                settings->setColorTid(ColorSchemes::BLACK);
                break;
            case 1:
                setColorScheme(ThemeStore::colorScheme(ColorSchemes::DARK));
                settings->setColorTid(ColorSchemes::DARK);
                break;
            case 2:
                setColorScheme(ThemeStore::colorScheme(ColorSchemes::DARKBLUE));
                settings->setColorTid(ColorSchemes::DARKBLUE);
                break;
            case 3:
                setColorScheme(ThemeStore::colorScheme(ColorSchemes::LIGHT));
                settings->setColorTid(ColorSchemes::LIGHT);
                break;
            default:;
            }
        }
    );

    connect(ui->useSystemColorsCheckBox, &QCheckBox::toggled, [this](bool useSystemTheme) {
        if (useSystemTheme) {
            ui->themeSelectorComboBox->setCurrentIndex(-1);
            setColorScheme(ThemeStore::colorScheme(ColorSchemes::SYSTEM));
            settings->setColorTid(ColorSchemes::SYSTEM);
        } else {
            readColorScheme();
            settings->setColorTid(ColorSchemes::CUSTOMIZED);
        }
        ui->themeSelectorComboBox->setEnabled(!useSystemTheme);
        ui->colorConfigSubgroup->setEnabled(!useSystemTheme);
        ui->modifySystemSchemeLabel->setVisible(useSystemTheme);
    });

    connect(ui->modifySystemSchemeLabel, &ClickableLabel::clicked, [this]() {
        ui->useSystemColorsCheckBox->setChecked(false);
        setColorScheme(ThemeStore::colorScheme(ColorSchemes::CUSTOMIZED));
        settings->setColorTid(ColorSchemes::CUSTOMIZED);
    });

    ui->colorSelectorAccent->setDescription(tr("Accent color"));
    ui->colorSelectorBackground->setDescription(tr("Windowed mode background"));
    ui->colorSelectorFullscreen->setDescription(tr("Fullscreen mode background"));
    ui->colorSelectorFolderview->setDescription(tr("FolderView background"));
    ui->colorSelectorFolderviewPanel->setDescription(tr("FolderView top panel"));
    ui->colorSelectorText->setDescription(tr("Text color"));
    ui->colorSelectorWidget->setDescription(tr("Widget background"));
    ui->colorSelectorWidgetBorder->setDescription(tr("Widget border"));
    ui->colorSelectorOverlay->setDescription(tr("Overlay background"));
    ui->colorSelectorOverlayText->setDescription(tr("Overlay text"));
    ui->colorSelectorScrollbar->setDescription(tr("Scrollbars"));

#ifndef USE_KDE_BLUR
    ui->blurBackgroundCheckBox->setEnabled(false);
#endif

#ifndef USE_MPV
    ui->videoPlaybackGroup->setEnabled(false);
    // ui->novideoInfoLabel->setHidden(false);
#else
    // ui->novideoInfoLabel->setHidden(true);
#endif

#ifdef USE_OPENCV
    ui->scalingQualityComboBox->addItem(QS("Bilinear+sharpen (OpenCV)"));
    ui->scalingQualityComboBox->addItem(QS("Bicubic (OpenCV)"));
    ui->scalingQualityComboBox->addItem(QS("Bicubic+sharpen (OpenCV)"));
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ui->memoryLimitSpinBox->setEnabled(false);
    ui->memoryLimitLabel->setEnabled(false);
#endif

    if (!settings->supportedFormats().contains("jxl"))
        ui->animatedJxlCheckBox->hide();

    setupSidebar();

    // setup radioBtn groups
    fitModeGrp.addButton(ui->fitModeWindow);
    fitModeGrp.addButton(ui->fitModeWidth);
    fitModeGrp.addButton(ui->fitMode1to1);
    folderEndGrp.addButton(ui->folderEndSwitchFolder);
    folderEndGrp.addButton(ui->folderEndNoAction);
    folderEndGrp.addButton(ui->folderEndLoop);
    zoomIndGrp.addButton(ui->zoomIndicatorAuto);
    zoomIndGrp.addButton(ui->zoomIndicatorOff);
    zoomIndGrp.addButton(ui->zoomIndicatorOn);

    // readable language names
    langs.insert(QS("de_DE"), QS("Deutsch"));
    langs.insert(QS("en_US"), QS("English"));
    langs.insert(QS("es_ES"), QS("Español"));
    langs.insert(QS("fr_FR"), QS("Français"));
    langs.insert(QS("uk_UA"), QS("Українська"));
    langs.insert(QS("zh_CN"), QS("简体中文"));
    // fill langs combobox, sorted by locale
    ui->langComboBox->addItems(langs.values());
    // insert system language entry manually at the beginning
    langs.insert(QS("system"), QS("System language"));
    ui->langComboBox->insertItem(0, QS("System language"));

    connect(this, &SettingsDialog::settingsChanged, settings, &Settings::sendChangeNotification);
    readSettings();

    adjustSizeToContents();
}
//------------------------------------------------------------------------------
SettingsDialog::~SettingsDialog()
{
    delete ui;
}
//------------------------------------------------------------------------------
// an attempt to force minimum width to fit contents
void SettingsDialog::adjustSizeToContents()
{
    // general tab
    ui->gridLayout->activate();
    ui->horizontalLayout_28->activate();
    ui->horizontalLayout_19->activate();
    ui->gridLayout_3->activate();
    ui->horizontalLayout_18->activate();
    ui->gridLayout_4->activate();
    ui->horizontalLayout_24->activate();
    ui->gridLayout_5->activate();
    ui->slideshowGroupContents->activate();
    ui->scrollAreaWidgetContents->layout()->activate();
    ui->scrollArea->setMinimumWidth(ui->scrollAreaWidgetContents->minimumSizeHint().width());
    // view tab
    ui->horizontalLayout_29->activate();
    ui->horizontalLayout_31->activate();
    ui->widget->layout()->activate();
    ui->scrollAreaWidgetContents_3->layout()->activate();
    ui->scrollArea_3->setMinimumWidth(ui->scrollAreaWidgetContents_3->minimumSizeHint().width());
    // container
    // ui->stackedWidget->layout()->activate();
    setMinimumWidth(sizeHint().width() + 22);

    // qDebug() << "window:" << this->sizeHint() << this->minimumSizeHint() << this->size();
    // qDebug() << "stackedwidget:" << ui->stackedWidget->sizeHint() << ui->stackedWidget->minimumSizeHint() <<
    // ui->stackedWidget->size(); qDebug() << "scrollarea:" << ui->scrollArea->sizeHint() << ui->scrollArea->minimumSizeHint() <<
    // ui->scrollArea->size(); qDebug() << "scrollareawidget:" << ui->scrollAreaWidgetContents->sizeHint() <<
    // ui->scrollAreaWidgetContents->minimumSizeHint() << ui->scrollAreaWidgetContents->size(); qDebug() << "grid" <<
    // ui->gridLayout_15->sizeHint(); qDebug() << "wtf" << ui->startInFolderViewCheckBox->sizeHint() <<
    // ui->startInFolderViewCheckBox->minimumSizeHint();
}
//------------------------------------------------------------------------------
void SettingsDialog::resetToDesktopTheme()
{
    settings->setColorScheme(ThemeStore::colorScheme(ColorSchemes::SYSTEM));
    readColorScheme();
}
//------------------------------------------------------------------------------
void SettingsDialog::setupSidebar()
{
}
//------------------------------------------------------------------------------
void SettingsDialog::readSettings()
{
    ui->loopSlideshowCheckBox->setChecked(settings->loopSlideshow());
    ui->videoPlaybackCheckBox->setChecked(settings->videoPlayback());
    ui->videoPlaybackGroupContents->setEnabled(settings->videoPlayback());
    ui->playSoundsCheckBox->setChecked(settings->playVideoSounds());
    ui->enablePanelCheckBox->setChecked(settings->panelEnabled());
    ui->thumbnailPanelGroupContents->setEnabled(settings->panelEnabled());
    ui->panelFullscreenOnlyCheckBox->setChecked(settings->panelFullscreenOnly());
    ui->squareThumbnailsCheckBox->setChecked(settings->squareThumbnails());
    ui->transparencyGridCheckBox->setChecked(settings->transparencyGrid());
    ui->enableSmoothScrollCheckBox->setChecked(settings->enableSmoothScroll());
    ui->usePreloaderCheckBox->setChecked(settings->usePreloader());
    ui->useThumbnailCacheCheckBox->setChecked(settings->useThumbnailCache());
    ui->smoothUpscalingCheckBox->setChecked(settings->smoothUpscaling());
    ui->expandImageCheckBox->setChecked(settings->expandImage());
    ui->expandImagesGroupContents->setEnabled(settings->expandImage());
    ui->smoothAnimatedImagesCheckBox->setChecked(settings->smoothAnimatedImages());
    ui->bgOpacitySlider->setValue(static_cast<int>(settings->backgroundOpacity() * 100));
    ui->blurBackgroundCheckBox->setChecked(settings->blurBackground());
    ui->sortingComboBox->setCurrentIndex(static_cast<int>(settings->sortingMode()));
    ui->confirmDeleteCheckBox->setChecked(settings->confirmDelete());
    ui->confirmTrashCheckBox->setChecked(settings->confirmTrash());
    ui->unlockMinZoomCheckBox->setChecked(settings->unlockMinZoom());
    ui->sortFoldersCheckBox->setChecked(settings->sortFolders());
    ui->trackpadDetectionCheckBox->setChecked(settings->trackpadDetection());

    if (settings->zoomIndicatorMode() == ZoomIndicatorMode::ENABLED)
        ui->zoomIndicatorOn->setChecked(true);
    else if (settings->zoomIndicatorMode() == ZoomIndicatorMode::AUTO)
        ui->zoomIndicatorAuto->setChecked(true);
    else
        ui->zoomIndicatorOff->setChecked(true);
    ui->showInfoBarFullscreen->setChecked(settings->infoBarFullscreen());
    ui->showInfoBarWindowed->setChecked(settings->infoBarWindowed());
    ui->showExtendedInfoTitle->setChecked(settings->windowTitleExtendedInfo());
    ui->cursorAutohideCheckBox->setChecked(settings->cursorAutohide());
    ui->keepFitModeCheckBox->setChecked(settings->keepFitMode());
    if (settings->focusPointIn1to1Mode() == ImageFocusPoint::TOP)
        ui->focus1to1Top->setChecked(true);
    else if (settings->focusPointIn1to1Mode() == ImageFocusPoint::CENTER)
        ui->focus1to1Center->setChecked(true);
    else
        ui->focus1to1Cursor->setChecked(true);
    ui->slideshowIntervalSpinBox->setValue(settings->slideshowInterval());
    ui->imageScrollingComboBox->setCurrentIndex(static_cast<int>(settings->imageScrolling()));
    ui->saveOverlayCheckBox->setChecked(settings->showSaveOverlay());
    ui->unloadThumbsCheckBox->setChecked(settings->unloadThumbs());
    if (settings->thumbPanelStyle() == ThumbPanelStyle::SIMPLE)
        ui->thumbStyleSimple->setChecked(true);
    else
        ui->thumbStyleExtended->setChecked(true);
    ui->animatedJxlCheckBox->setChecked(settings->jxlAnimation());
    ui->autoResizeWindowCheckBox->setChecked(settings->autoResizeWindow());
    ui->panelCenterSelectionCheckBox->setChecked(settings->panelCenterSelection());
    ui->useFixedZoomLevelsCheckBox->setChecked(settings->useFixedZoomLevels());
    ui->zoomLevels->setText(settings->zoomLevels());

    if (settings->defaultViewMode() == ViewMode::FOLDERVIEW)
        ui->startInFolderViewCheckBox->setChecked(true);
    else
        ui->startInFolderViewCheckBox->setChecked(false);

    if (settings->folderEndAction() == FolderEndAction::NOTHING)
        ui->folderEndNoAction->setChecked(true);
    else if (settings->folderEndAction() == FolderEndAction::LOOP)
        ui->folderEndLoop->setChecked(true);
    else
        ui->folderEndSwitchFolder->setChecked(true);

    ui->mpvLineEdit->setText(settings->mpvBinary());

    ui->zoomStepSlider->setValue(static_cast<int>(settings->zoomStep() * 100.f));
    onZoomStepSliderChanged(ui->zoomStepSlider->value());

    ui->autoResizeLimitSlider->setValue(static_cast<int>(settings->autoResizeLimit() / 5.0));
    onAutoResizeLimitSliderChanged(ui->autoResizeLimitSlider->value());

    ui->JPEGQualitySlider->setValue(settings->JPEGSaveQuality());
    onJPEGQualitySliderChanged(ui->JPEGQualitySlider->value());

    ui->expandLimitSlider->setValue(settings->expandLimit());
    onExpandLimitSliderChanged(ui->expandLimitSlider->value());

    // thumbnailer threads
    ui->thumbnailerThreadsSlider->setValue(settings->thumbnailerThreadCount());
    onThumbnailerThreadsSliderChanged(ui->thumbnailerThreadsSlider->value());

    ui->memoryLimitSpinBox->setValue(settings->memoryAllocationLimit());

    // language
    QString langName = langs.value(settings->language());
    if (langName.isEmpty() || ui->langComboBox->findText(langName) == -1)
        ui->langComboBox->setCurrentText(QS("en_US"));
    else
        ui->langComboBox->setCurrentText(langName);

    // ##### fit mode #####
    if (settings->imageFitMode() == ImageFitMode::WINDOW)
        ui->fitModeWindow->setChecked(true);
    else if (settings->imageFitMode() == ImageFitMode::WIDTH)
        ui->fitModeWidth->setChecked(true);
    else
        ui->fitMode1to1->setChecked(true);

    // ##### UI #####
    ui->scalingQualityComboBox->setCurrentIndex(static_cast<int>(settings->scalingFilter()));
    ui->fullscreenCheckBox->setChecked(settings->fullscreenMode());
    ui->pinPanelCheckBox->setChecked(settings->panelPinned());
    ui->panelPositionComboBox->setCurrentIndex(static_cast<int>(settings->panelPosition()));

    // reduce by 10x to have nice granular control in qslider
    ui->panelSizeSlider->setValue(settings->panelPreviewsSize() / 10);

    ui->useSystemColorsCheckBox->setChecked(settings->useSystemColorScheme());
    ui->modifySystemSchemeLabel->setVisible(settings->useSystemColorScheme());
    ui->themeSelectorComboBox->setEnabled(!settings->useSystemColorScheme());
    ui->colorConfigSubgroup->setEnabled(!settings->useSystemColorScheme());

    readColorScheme();
    readShortcuts();
    readScripts();
}
//------------------------------------------------------------------------------
void SettingsDialog::saveSettings()
{
    // wait for all background stuff to finish
    if (QThreadPool::globalInstance()->activeThreadCount())
        QThreadPool::globalInstance()->waitForDone();

    settings->setLoopSlideshow(ui->loopSlideshowCheckBox->isChecked());
    settings->setFullscreenMode(ui->fullscreenCheckBox->isChecked());
    if (ui->fitModeWindow->isChecked())
        settings->setImageFitMode(ImageFitMode::WINDOW);
    else if (ui->fitModeWidth->isChecked())
        settings->setImageFitMode(ImageFitMode::WIDTH);
    else
        settings->setImageFitMode(ImageFitMode::ORIGINAL);

    settings->setLanguage(langs.key(ui->langComboBox->currentText()));

    settings->setVideoPlayback(ui->videoPlaybackCheckBox->isChecked());
    settings->setPlayVideoSounds(ui->playSoundsCheckBox->isChecked());
    settings->setPanelEnabled(ui->enablePanelCheckBox->isChecked());
    settings->setPanelFullscreenOnly(ui->panelFullscreenOnlyCheckBox->isChecked());
    settings->setSquareThumbnails(ui->squareThumbnailsCheckBox->isChecked());
    settings->setTransparencyGrid(ui->transparencyGridCheckBox->isChecked());
    settings->setEnableSmoothScroll(ui->enableSmoothScrollCheckBox->isChecked());
    settings->setUsePreloader(ui->usePreloaderCheckBox->isChecked());
    settings->setUseThumbnailCache(ui->useThumbnailCacheCheckBox->isChecked());
    settings->setSmoothUpscaling(ui->smoothUpscalingCheckBox->isChecked());
    settings->setExpandImage(ui->expandImageCheckBox->isChecked());
    settings->setSmoothAnimatedImages(ui->smoothAnimatedImagesCheckBox->isChecked());

    settings->setBackgroundOpacity(ui->bgOpacitySlider->value() / 100.0);
    settings->setBlurBackground(ui->blurBackgroundCheckBox->isChecked());
    settings->setSortingMode(static_cast<SortingMode>(ui->sortingComboBox->currentIndex()));
    settings->setConfirmDelete(ui->confirmDeleteCheckBox->isChecked());
    settings->setConfirmTrash(ui->confirmTrashCheckBox->isChecked());
    settings->setUnlockMinZoom(ui->unlockMinZoomCheckBox->isChecked());
    settings->setSortFolders(ui->sortFoldersCheckBox->isChecked());
    settings->setTrackpadDetection(ui->trackpadDetectionCheckBox->isChecked());

    if (ui->zoomIndicatorOn->isChecked())
        settings->setZoomIndicatorMode(ZoomIndicatorMode::ENABLED);
    else if (ui->zoomIndicatorAuto->isChecked())
        settings->setZoomIndicatorMode(ZoomIndicatorMode::AUTO);
    else
        settings->setZoomIndicatorMode(ZoomIndicatorMode::DISABLED);
    settings->setInfoBarFullscreen(ui->showInfoBarFullscreen->isChecked());
    settings->setInfoBarWindowed(ui->showInfoBarWindowed->isChecked());
    settings->setWindowTitleExtendedInfo(ui->showExtendedInfoTitle->isChecked());
    settings->setCursorAutohide(ui->cursorAutohideCheckBox->isChecked());
    settings->setKeepFitMode(ui->keepFitModeCheckBox->isChecked());
    if (ui->focus1to1Top->isChecked())
        settings->setFocusPointIn1to1Mode(ImageFocusPoint::TOP);
    else if (ui->focus1to1Center->isChecked())
        settings->setFocusPointIn1to1Mode(ImageFocusPoint::CENTER);
    else
        settings->setFocusPointIn1to1Mode(ImageFocusPoint::CURSOR);

    settings->setSlideshowInterval(ui->slideshowIntervalSpinBox->value());

    if (ui->startInFolderViewCheckBox->isChecked())
        settings->setDefaultViewMode(ViewMode::FOLDERVIEW);
    else
        settings->setDefaultViewMode(ViewMode::DOCUMENT);

    if (ui->folderEndNoAction->isChecked())
        settings->setFolderEndAction(FolderEndAction::NOTHING);
    else if (ui->folderEndLoop->isChecked())
        settings->setFolderEndAction(FolderEndAction::LOOP);
    else
        settings->setFolderEndAction(FolderEndAction::GOTO_ADJACENT);

    settings->setMpvBinary(ui->mpvLineEdit->text());
    settings->setScalingFilter(static_cast<ScalingFilter>(ui->scalingQualityComboBox->currentIndex()));
    settings->setImageScrolling(static_cast<ImageScrolling>(ui->imageScrollingComboBox->currentIndex()));
    settings->setShowSaveOverlay(ui->saveOverlayCheckBox->isChecked());
    settings->setUnloadThumbs(ui->unloadThumbsCheckBox->isChecked());
    if (ui->thumbStyleSimple->isChecked())
        settings->setThumbPanelStyle(ThumbPanelStyle::SIMPLE);
    else
        settings->setThumbPanelStyle(ThumbPanelStyle::EXTENDED);
    settings->setJxlAnimation(ui->animatedJxlCheckBox->isChecked());
    settings->setAutoResizeWindow(ui->autoResizeWindowCheckBox->isChecked());
    settings->setPanelCenterSelection(ui->panelCenterSelectionCheckBox->isChecked());
    settings->setUseFixedZoomLevels(ui->useFixedZoomLevelsCheckBox->isChecked());
    settings->setZoomLevels(ui->zoomLevels->text());

    settings->setPanelPinned(ui->pinPanelCheckBox->isChecked());
    int panelPos = ui->panelPositionComboBox->currentIndex();
    settings->setPanelPosition(static_cast<PanelPosition>(panelPos));

    settings->setPanelPreviewsSize(ui->panelSizeSlider->value() * 10);

    settings->setJPEGSaveQuality(ui->JPEGQualitySlider->value());
    settings->setZoomStep(ui->zoomStepSlider->value() / 100.0);
    settings->setAutoResizeLimit(ui->autoResizeLimitSlider->value() * 5);
    settings->setExpandLimit(ui->expandLimitSlider->value());
    settings->setThumbnailerThreadCount(ui->thumbnailerThreadsSlider->value());
    settings->setMemoryAllocationLimit(ui->memoryLimitSpinBox->value());

    settings->setUseSystemColorScheme(ui->useSystemColorsCheckBox->isChecked());

    saveColorScheme();
    saveShortcuts();

    scriptManager->saveScripts();
    actionManager->saveShortcuts();
    emit settingsChanged();
}
//------------------------------------------------------------------------------
void SettingsDialog::saveSettingsAndClose()
{
    saveSettings();
    this->close();
}
//------------------------------------------------------------------------------
void SettingsDialog::readColorScheme()
{
    auto colors = settings->colorScheme();
    setColorScheme(colors);
}

void SettingsDialog::setColorScheme(ColorScheme colors) const
{
    switch (static_cast<ColorSchemes>(colors.tid)) {
    case ColorSchemes::LIGHT:
        ui->themeSelectorComboBox->setCurrentIndex(3);
        break;
    case ColorSchemes::BLACK:
        ui->themeSelectorComboBox->setCurrentIndex(0);
        break;
    case ColorSchemes::DARK:
        ui->themeSelectorComboBox->setCurrentIndex(1);
        break;
    case ColorSchemes::DARKBLUE:
        ui->themeSelectorComboBox->setCurrentIndex(2);
        break;
    default:
        ui->themeSelectorComboBox->setCurrentIndex(-1);
        break;
    }
    ui->colorSelectorAccent->setColor(colors.accent);
    ui->colorSelectorBackground->setColor(colors.background);
    ui->colorSelectorFullscreen->setColor(colors.background_fullscreen);
    ui->colorSelectorFolderview->setColor(colors.folderview);
    ui->colorSelectorFolderviewPanel->setColor(colors.folderview_topbar);
    ui->colorSelectorText->setColor(colors.text);
    ui->colorSelectorIcons->setColor(colors.icons);
    ui->colorSelectorWidget->setColor(colors.widget);
    ui->colorSelectorWidgetBorder->setColor(colors.widget_border);
    ui->colorSelectorOverlay->setColor(colors.overlay);
    ui->colorSelectorOverlayText->setColor(colors.overlay_text);
    ui->colorSelectorScrollbar->setColor(colors.scrollbar);
}

//------------------------------------------------------------------------------
void SettingsDialog::saveColorScheme() const
{
    BaseColorScheme base;
    base.accent                = ui->colorSelectorAccent->color();
    base.background            = ui->colorSelectorBackground->color();
    base.background_fullscreen = ui->colorSelectorFullscreen->color();
    base.folderview            = ui->colorSelectorFolderview->color();
    base.folderview_topbar     = ui->colorSelectorFolderviewPanel->color();
    base.text                  = ui->colorSelectorText->color();
    base.icons                 = ui->colorSelectorIcons->color();
    base.widget                = ui->colorSelectorWidget->color();
    base.widget_border         = ui->colorSelectorWidgetBorder->color();
    base.overlay               = ui->colorSelectorOverlay->color();
    base.overlay_text          = ui->colorSelectorOverlayText->color();
    base.scrollbar             = ui->colorSelectorScrollbar->color();
    base.tid                   = settings->colorScheme().tid;
    settings->setColorScheme(ColorScheme(base));
}
//------------------------------------------------------------------------------
void SettingsDialog::readShortcuts() const
{
    ui->shortcutsTableWidget->clearContents();
    ui->shortcutsTableWidget->setRowCount(0);
    QMap<QString, QString> const   shortcuts = actionManager->allShortcuts();
    QMapIterator<QString, QString> i(shortcuts);
    while (i.hasNext()) {
        i.next();
        addShortcutToTable(i.value(), i.key());
    }
}
//------------------------------------------------------------------------------
void SettingsDialog::readScripts() const
{
    ui->scriptsListWidget->clear();
    QMap<QString, Script> const   scripts = scriptManager->allScripts();
    QMapIterator<QString, Script> i(scripts);
    while (i.hasNext()) {
        i.next();
        addScriptToList(i.key());
    }
}
//------------------------------------------------------------------------------
// does not check if the shortcut already there
void SettingsDialog::addScriptToList(QString const &name) const
{
    if (name.isEmpty())
        return;

    QListWidget     *list     = ui->scriptsListWidget;
    QListWidgetItem *nameItem = new QListWidgetItem(name);
    nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    list->insertItem(ui->scriptsListWidget->count(), nameItem);
    list->sortItems(Qt::AscendingOrder);
}
//------------------------------------------------------------------------------
void SettingsDialog::addScript()
{
    ScriptEditorDialog w;
    if (w.exec()) {
        if (w.scriptName().isEmpty())
            return;
        scriptManager->addScript(w.scriptName(), w.script());
        readScripts();
    }
}
//------------------------------------------------------------------------------
void SettingsDialog::editScript()
{
    int row = ui->scriptsListWidget->currentRow();
    if (row >= 0) {
        QString name = ui->scriptsListWidget->currentItem()->text();
        editScript(name);
    }
}
//------------------------------------------------------------------------------
void SettingsDialog::editScript(QListWidgetItem *item)
{
    if (item)
        editScript(item->text());
}
//------------------------------------------------------------------------------
void SettingsDialog::editScript(QString const &name)
{
    ScriptEditorDialog w(name, scriptManager->getScript(name));
    if (w.exec()) {
        if (w.scriptName().isEmpty())
            return;
        scriptManager->addScript(w.scriptName(), w.script());
        readScripts();
    }
}
//------------------------------------------------------------------------------
void SettingsDialog::removeScript()
{
    int row = ui->scriptsListWidget->currentRow();
    if (row >= 0) {
        QString scriptName = ui->scriptsListWidget->currentItem()->text();
        delete ui->scriptsListWidget->takeItem(row);
        saveShortcuts();
        actionManager->removeAllShortcuts(QS("s:") + scriptName);
        readShortcuts();
        scriptManager->removeScript(scriptName);
    }
}
//------------------------------------------------------------------------------
// does not check if the shortcut already there
void SettingsDialog::addShortcutToTable(QString const &action, QString const &shortcut) const
{
    if (action.isEmpty() || shortcut.isEmpty())
        return;

    ui->shortcutsTableWidget->setRowCount(ui->shortcutsTableWidget->rowCount() + 1);
    auto *actionItem = new QTableWidgetItem(action);
    actionItem->setTextAlignment(Qt::AlignCenter);
    ui->shortcutsTableWidget->setItem(ui->shortcutsTableWidget->rowCount() - 1, 0, actionItem);
    auto *shortcutItem = new QTableWidgetItem(shortcut);
    shortcutItem->setTextAlignment(Qt::AlignCenter);
    ui->shortcutsTableWidget->setItem(ui->shortcutsTableWidget->rowCount() - 1, 1, shortcutItem);
    // EFFICIENCY
    ui->shortcutsTableWidget->sortByColumn(0, Qt::AscendingOrder);
}
//------------------------------------------------------------------------------
void SettingsDialog::addShortcut()
{
    ShortcutCreatorDialog w;
    if (!w.exec())
        return;
    for (int i = 0; i < ui->shortcutsTableWidget->rowCount(); i++)
        if (ui->shortcutsTableWidget->item(i, 1)->text() == w.selectedShortcut())
            removeShortcutAt(i);
    addShortcutToTable(w.selectedAction(), w.selectedShortcut());
    // select
    auto items = ui->shortcutsTableWidget->findItems(w.selectedShortcut(), Qt::MatchExactly);
    if (items.count()) {
        int newRow = ui->shortcutsTableWidget->row(items.at(0));
        ui->shortcutsTableWidget->selectRow(newRow);
    }
}
//------------------------------------------------------------------------------
void SettingsDialog::removeShortcutAt(int row) const
{
    if (row > 0 && row >= ui->shortcutsTableWidget->rowCount())
        return;
    ui->shortcutsTableWidget->removeRow(row);
}
//------------------------------------------------------------------------------
void SettingsDialog::editShortcut(int row) const
{
    if (row >= 0) {
        ShortcutCreatorDialog w;
        w.setWindowTitle(tr("Edit shortcut"));
        w.setAction(ui->shortcutsTableWidget->item(row, 0)->text());
        w.setShortcut(ui->shortcutsTableWidget->item(row, 1)->text());
        if (!w.exec())
            return;
        // remove itself
        removeShortcutAt(row);
        // remove anything we are replacing
        for (int i = 0; i < ui->shortcutsTableWidget->rowCount(); i++)
            if (ui->shortcutsTableWidget->item(i, 1)->text() == w.selectedShortcut())
                removeShortcutAt(i);
        // re-add
        addShortcutToTable(w.selectedAction(), w.selectedShortcut());
        // re-select
        auto items = ui->shortcutsTableWidget->findItems(w.selectedShortcut(), Qt::MatchExactly);
        if (items.count()) {
            int newRow = ui->shortcutsTableWidget->row(items.at(0));
            ui->shortcutsTableWidget->selectRow(newRow);
        }
    }
}
//------------------------------------------------------------------------------
void SettingsDialog::editShortcut()
{
    editShortcut(ui->shortcutsTableWidget->currentRow());
}
//------------------------------------------------------------------------------
void SettingsDialog::removeShortcut() const
{
    removeShortcutAt(ui->shortcutsTableWidget->currentRow());
}
//------------------------------------------------------------------------------
void SettingsDialog::saveShortcuts() const
{
    actionManager->removeAllShortcuts();
    for (int i = 0; i < ui->shortcutsTableWidget->rowCount(); i++)
        actionManager->addShortcut(ui->shortcutsTableWidget->item(i, 1)->text(), ui->shortcutsTableWidget->item(i, 0)->text());
}
//------------------------------------------------------------------------------
void SettingsDialog::resetShortcuts() const
{
    actionManager->resetDefaults();
    readShortcuts();
}
//------------------------------------------------------------------------------
void SettingsDialog::resetZoomLevels() const
{
    ui->zoomLevels->setText(settings->defaultZoomLevels());
}
//------------------------------------------------------------------------------
void SettingsDialog::selectMpvPath()
{
    QFileDialog dialog;
    QString     file = dialog.getOpenFileName(this, tr("Navigate to mpv binary"), QString(), QS("mpv*"));
    if (!file.isEmpty())
        ui->mpvLineEdit->setText(file);
}
//------------------------------------------------------------------------------
void SettingsDialog::onExpandLimitSliderChanged(int value) const
{
    if (value == 0)
        ui->expandLimitLabel->setText(QS("-"));
    else
        ui->expandLimitLabel->setText(QString::number(value) + u'x');
}
//------------------------------------------------------------------------------
void SettingsDialog::onJPEGQualitySliderChanged(int value) const
{
    ui->JPEGQualityLabel->setText(QString::number(value) + u'%');
}
//------------------------------------------------------------------------------
void SettingsDialog::onZoomStepSliderChanged(int value) const
{
    ui->zoomStepLabel->setText(QString::number(value / 100.0, 'f', 2) + u'x');
}
//------------------------------------------------------------------------------
void SettingsDialog::onThumbnailerThreadsSliderChanged(int value) const
{
    ui->thumbnailerThreadsLabel->setText(QString::number(value));
}
//------------------------------------------------------------------------------
void SettingsDialog::onBgOpacitySliderChanged(int value) const
{
    ui->bgOpacityPercentLabel->setText(QString::number(value) + u'%');
}
//------------------------------------------------------------------------------
void SettingsDialog::onAutoResizeLimitSliderChanged(int value) const
{
    ui->autoResizeLimit->setText(QString::number(value * 5.0, 'f', 0) + u'%');
}
//------------------------------------------------------------------------------
int SettingsDialog::exec()
{
    return QDialog::exec();
}

void SettingsDialog::switchToPage(int number) const
{
    ui->sideBar2->selectEntry(number);
}
