#include "contextmenu.h"
#include "ui_contextmenu.h"

ContextMenu::ContextMenu(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ContextMenu)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowType::Popup | Qt::WindowType::WindowStaysOnTopHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground, true);
    setAttribute(Qt::WidgetAttribute::WA_NoMousePropagation, true);
    this->hide();

    // -------------------------------------------------------------------------
    // setup actions
    // top zoom buttons
    ui->zoomIn->setAction(QS("zoomIn"));
    ui->zoomIn->setIconPath(QS(":/res/icons/common/buttons/contextmenu/zoom-in18.png"));
    ui->zoomIn->setTriggerMode(TriggerMode::Press);
    ui->zoomOut->setAction(QS("zoomOut"));
    ui->zoomOut->setIconPath(QS(":/res/icons/common/buttons/contextmenu/zoom-out18.png"));
    ui->zoomOut->setTriggerMode(TriggerMode::Press);
    ui->zoomOriginal->setAction(QS("fitNormal"));
    ui->zoomOriginal->setIconPath(QS(":/res/icons/common/buttons/contextmenu/zoom-original18.png"));
    ui->zoomOriginal->setTriggerMode(TriggerMode::Press);
    ui->fitWidth->setAction(QS("fitWidth"));
    ui->fitWidth->setIconPath(QS(":/res/icons/common/buttons/contextmenu/fit-width18.png"));
    ui->fitWidth->setTriggerMode(TriggerMode::Press);
    ui->fitWindow->setAction(QS("fitWindow"));
    ui->fitWindow->setIconPath(QS(":/res/icons/common/buttons/contextmenu/fit-window18.png"));
    ui->fitWindow->setTriggerMode(TriggerMode::Press);
    // -------------------------------------------------------------------------
    // transform buttons
    ui->rotateLeft->setAction(QS("rotateLeft"));
    ui->rotateLeft->setIconPath(QS(":/res/icons/common/menuitem/rotate-left16.png"));
    ui->rotateLeft->setTriggerMode(TriggerMode::Press);
    ui->rotateRight->setAction(QS("rotateRight"));
    ui->rotateRight->setIconPath(QS(":/res/icons/common/menuitem/rotate-right16.png"));
    ui->rotateRight->setTriggerMode(TriggerMode::Press);
    ui->flipH->setAction(QS("flipH"));
    ui->flipH->setIconPath(QS(":/res/icons/common/menuitem/flip-h16.png"));
    ui->flipH->setTriggerMode(TriggerMode::Press);
    ui->flipV->setAction(QS("flipV"));
    ui->flipV->setIconPath(QS(":/res/icons/common/menuitem/flip-v16.png"));
    ui->flipV->setTriggerMode(TriggerMode::Press);
    ui->crop->setAction(QS("crop"));
    ui->crop->setIconPath(QS(":/res/icons/common/menuitem/image-crop16.png"));
    ui->crop->setTriggerMode(TriggerMode::Press);
    ui->resize->setAction(QS("resize"));
    ui->resize->setIconPath(QS(":/res/icons/common/menuitem/resize16.png"));
    ui->resize->setTriggerMode(TriggerMode::Press);
    // -------------------------------------------------------------------------
    //  entries
    ui->print->setAction(QS("print"));
    ui->print->setText(tr("Print"));
    ui->print->setIconPath(QS(":/res/icons/common/menuitem/print16.png"));

    ui->copy->setAction(QS("copyFile"));
    ui->copy->setText(tr("Quick copy"));
    ui->copy->setIconPath(QS(":/res/icons/common/menuitem/copy16.png"));

    ui->move->setAction(QS("moveFile"));
    ui->move->setText(tr("Quick move"));
    ui->move->setIconPath(QS(":/res/icons/common/menuitem/move16.png"));

    ui->trash->setAction(QS("moveToTrash"));
    ui->trash->setText(tr("Move to trash"));
    ui->trash->setIconPath(QS(":/res/icons/common/menuitem/trash16.png"));
    ui->trash->setShortcutText(QS(""));
    // -------------------------------------------------------------------------
    ui->open->setAction(QS("open"));
    ui->open->setText(tr("Open"));
    ui->open->setIconPath(QS(":/res/icons/common/menuitem/open16.png"));
    ui->open->setShortcutText(QS(""));

    ui->folderView->setAction(QS("folderView"));
    ui->folderView->setText(tr("Folder View"));
    ui->folderView->setIconPath(QS(":/res/icons/common/menuitem/folderview16.png"));
    ui->folderView->setShortcutText(QS(""));

    ui->settings->setAction(QS("openSettings"));
    ui->settings->setText(tr("Settings"));
    ui->settings->setIconPath(QS(":/res/icons/common/menuitem/settings16.png"));
    // -------------------------------------------------------------------------
    ui->openWith->setText(tr("Open with..."));
    ui->openWith->setIconPath(QS(":/res/icons/common/menuitem/run16.png"));
    ui->openWith->setPassthroughClicks(false);
    connect(ui->openWith, &ContextMenuItem::pressed, this, &ContextMenu::switchToScriptsPage);
    // -------------------------------------------------------------------------
    ui->showLocation->setAction(QS("showInDirectory"));
    ui->showLocation->setText(tr("Show in folder"));
    ui->showLocation->setIconPath(QS(":/res/icons/common/menuitem/folder16.png"));
    // -------------------------------------------------------------------------
    // force resize to fit new menuitem width
    this->adjustSize();

    // Scripts page
    // -------------------------------------------------------------------------
    ui->backButton->setText(tr("Back"));
    ui->backButton->setIconPath(QS(":/res/icons/common/menuitem/back16.png"));
    ui->backButton->setPassthroughClicks(false);
    ui->scriptSetupButton->setText(tr("Configure menu"));
    ui->scriptSetupButton->setIconPath(QS(":/res/icons/common/menuitem/settings16.png"));
    connect(ui->backButton, &ContextMenuItem::pressed, this, &ContextMenu::switchToMainPage);
    connect(ui->scriptSetupButton, &ContextMenuItem::pressed, this, &ContextMenu::showScriptSettings);
    fillOpenWithMenu();
}

ContextMenu::~ContextMenu()
{
    delete ui;
}

void ContextMenu::fillOpenWithMenu()
{
    auto const &scripts = ScriptManager::allScripts();
    for (auto const &[key, val] : scripts.asKeyValueRange()) {
        if (!val.command.isEmpty()) {
            auto btn = new ContextMenuItem();
            btn->setAction(QS("s:") + key);
            btn->setIconPath(QS(":/res/icons/common/menuitem/open16.png"));
            btn->setText(key);
            ui->scriptsLayout->addWidget(btn);
        }
    }
}

void ContextMenu::switchToMainPage()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void ContextMenu::switchToScriptsPage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void ContextMenu::setImageEntriesEnabled(bool mode)
{
    ui->rotateLeft->setEnabled(mode);
    ui->rotateRight->setEnabled(mode);
    ui->flipH->setEnabled(mode);
    ui->flipV->setEnabled(mode);
    ui->crop->setEnabled(mode);
    ui->resize->setEnabled(mode);

    ui->copy->setEnabled(mode);
    ui->move->setEnabled(mode);
    ui->trash->setEnabled(mode);
    ui->openWith->setEnabled(mode);
    ui->showLocation->setEnabled(mode);
}

void ContextMenu::showAt(QPoint pos)
{
    switchToMainPage();
    QRect geom = geometry();
    geom.moveTopLeft(pos);
    setGeometry(geom);
    show();
}

void ContextMenu::setGeometry(QRect geom)
{
    auto screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) // fallback
        screen = QGuiApplication::primaryScreen();
    if (screen) {
        // fit inside the current screen
        if (geom.bottom() > screen->geometry().bottom())
            geom.moveBottom(QCursor::pos().y());
        if (geom.right() > screen->geometry().right())
            geom.moveRight(screen->geometry().right());
    }
    QWidget::setGeometry(geom);
}

void
ContextMenu::show()
{
    QWidget::show();
}

void
ContextMenu::hide()
{
    QWidget::hide();
}

void ContextMenu::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    hide();
}

void ContextMenu::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ContextMenu::keyPressEvent(QKeyEvent *event)
{
    quint32 nativeScanCode = event->nativeScanCode();
    QString key            = actionManager->keyForNativeScancode(nativeScanCode);

    // todo: keyboard navigation
    if (key == QSV("Up")) {
    }
    if (key == QSV("Down")) {
    }
    if (key == QSV("Esc")) {
        hide();
    }
    if (key == QSV("Enter")) {
    } else {
        actionManager->processEvent(event);
    }
}
