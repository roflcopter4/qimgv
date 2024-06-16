#include "CopyOverlay.h"
#include "ui_CopyOverlay.h"

CopyOverlay::CopyOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::CopyOverlay),
      mode(CopyOverlayMode::COPY)
{
    ui->setupUi(this);
    hide();
    setFadeEnabled(true);

    ui->closeButton->setIconPath(QS(":/res/icons/common/overlay/close-dim16.png"));
    ui->headerIcon->setIconPath(QS(":/res/icons/common/overlay/copy16.png"));
    ui->headerLabel->setText(tr("Copy to..."));

    createShortcuts();

    paths = settings->savedPaths();
    if (paths.count() < maxPathCount)
        createDefaultPaths();
    createPathWidgets();

    setAcceptKeyboardFocus(true);

    if (parent)
        setContainerSize(parent->size());

    readSettings();
    connect(settings, &Settings::settingsChanged, this, &CopyOverlay::readSettings);
}

CopyOverlay::~CopyOverlay()
{
    delete ui;
}

void CopyOverlay::show()
{
    OverlayWidget::show();
    setFocus();
}

void CopyOverlay::hide()
{
    OverlayWidget::hide();
}

void CopyOverlay::setDialogMode(CopyOverlayMode _mode)
{
    mode = _mode;
    if (mode == CopyOverlayMode::COPY) {
        ui->headerIcon->setIconPath(QS(":/res/icons/common/overlay/copy16.png"));
        ui->headerLabel->setText(tr("Copy to..."));
    } else {
        ui->headerIcon->setIconPath(QS(":/res/icons/common/overlay/move16.png"));
        ui->headerLabel->setText(tr("Move to..."));
    }
}

CopyOverlayMode CopyOverlay::operationMode() const
{
    return mode;
}

void CopyOverlay::removePathWidgets()
{
    for (int i = 0; i < pathWidgets.count(); ++i) {
        QWidget *tmp = pathWidgets[i];
        ui->pathSelectorsLayout->removeWidget(tmp);
        delete tmp;
    }
    pathWidgets.clear();
}

void CopyOverlay::createPathWidgets()
{
    removePathWidgets();
    int count = paths.length() > maxPathCount ? maxPathCount : int(paths.length());
    for (int i = 0; i < count; ++i) {
        auto *item = new PathSelectorMenuItem(this);
        item->setDirectory(paths[i]);
        item->setShortcutText(shortcuts.key(i));
        connect(item, &PathSelectorMenuItem::directorySelected, this, &CopyOverlay::requestFileOperation);
        pathWidgets.append(item);
        ui->pathSelectorsLayout->addWidget(item);
    }
}

void CopyOverlay::createShortcuts()
{
    for (int i = 0; i < maxPathCount; ++i)
        shortcuts.insert(QString::number(i + 1), i);
}

void CopyOverlay::requestFileOperation(QString const &path)
{
    if (mode == CopyOverlayMode::COPY)
        emit copyRequested(path);
    else
        emit moveRequested(path);
}

void CopyOverlay::readSettings()
{
    // don't interfere with the main panel
    if (settings->panelEnabled() && settings->panelPosition() == PanelPosition::BOTTOM)
        setPosition(FloatingWidgetPosition::TOPLEFT);
    else
        setPosition(FloatingWidgetPosition::BOTTOMLEFT);
    update();
}

// for some reason, duplicate folders may appear in the configuration
// we remove duplicate directories
void CopyOverlay::saveSettings()
{
    paths.clear();
    QStringList temp;
    for (int i = 0; i < pathWidgets.count(); i++) {
        QString path = pathWidgets[i]->path();
        if (!path.isEmpty()) {
            if (!temp.contains(path)) {
                temp << path;
                paths << pathWidgets[i]->directory();
            }
        }
    }
    settings->setSavedPaths(paths);
}

void CopyOverlay::createDefaultPaths()
{
    QString home = QDir::homePath();
    if (paths.count() < 1 || paths[0].isEmpty() || paths[0][0] == u'@') {
        paths.clear();
        paths << home;
    }
    if (paths.count() != 1 || paths[0] != home)
        return;
    QDir dir(home);

    for (QFileInfo const &mfi : dir.entryInfoList()) {
        if (paths.count() >= maxPathCount)
            break;
        if (mfi.isFile())
            continue;
        QString fname = mfi.fileName();
        if (fname == u"."_sv ||
            fname == u".."_sv ||
            // hide directory
            fname[0] == u'.' ||
            // windows system directory
            fname == u"3D Objects"_sv ||
            fname == u"Contacts"_sv ||
            fname == u"Favorites"_sv ||
            fname == u"Links"_sv ||
            fname == u"Saved Games"_sv ||
            fname == u"Searches"_sv)
        {
            continue;
        }

        QString   qpath(home + u'/' + mfi.fileName());
        QFileInfo qinfo(qpath);
        if (qinfo.permission(QFile::WriteUser | QFile::ReadGroup))
            paths << qpath;
    }
}

// block native tab-switching so we can use it in shortcuts
bool CopyOverlay::focusNextPrevChild(bool)
{
    return false;
}

void CopyOverlay::keyPressEvent(QKeyEvent *event)
{
    event->accept();


    QString key = ActionManager::keyForNativeScancode(event->nativeScanCode());
    if (shortcuts.contains(key))
        requestFileOperation(pathWidgets[shortcuts[key]]->directory());
    else
        actionManager->processEvent(event);
}
