#include "ContextMenu.h"
#include "ui_ContextMenu.h"

ContextMenu::ContextMenu(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ContextMenu)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowType::Popup | Qt::WindowType::WindowStaysOnTopHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground);
    setAttribute(Qt::WidgetAttribute::WA_NoMousePropagation);
    hide();

    // -------------------------------------------------------------------------
    // setup actions
    // top zoom buttons
    ui->zoomIn->setAction(u"zoomIn"_s);
    ui->zoomIn->setIconPath(u":/res/icons/common/buttons/contextmenu/zoom-in18.png"_s);
    ui->zoomIn->setTriggerMode(TriggerMode::Press);
    ui->zoomOut->setAction(u"zoomOut"_s);
    ui->zoomOut->setIconPath(u":/res/icons/common/buttons/contextmenu/zoom-out18.png"_s);
    ui->zoomOut->setTriggerMode(TriggerMode::Press);
    ui->zoomOriginal->setAction(u"fitNormal"_s);
    ui->zoomOriginal->setIconPath(u":/res/icons/common/buttons/contextmenu/zoom-original18.png"_s);
    ui->zoomOriginal->setTriggerMode(TriggerMode::Press);
    ui->fitWidth->setAction(u"fitWidth"_s);
    ui->fitWidth->setIconPath(u":/res/icons/common/buttons/contextmenu/fit-width18.png"_s);
    ui->fitWidth->setTriggerMode(TriggerMode::Press);
    ui->fitWindow->setAction(u"fitWindow"_s);
    ui->fitWindow->setIconPath(u":/res/icons/common/buttons/contextmenu/fit-window18.png"_s);
    ui->fitWindow->setTriggerMode(TriggerMode::Press);
    // -------------------------------------------------------------------------
    // transform buttons
    ui->rotateLeft->setAction(u"rotateLeft"_s);
    ui->rotateLeft->setIconPath(u":/res/icons/common/menuitem/rotate-left16.png"_s);
    ui->rotateLeft->setTriggerMode(TriggerMode::Press);
    ui->rotateRight->setAction(u"rotateRight"_s);
    ui->rotateRight->setIconPath(u":/res/icons/common/menuitem/rotate-right16.png"_s);
    ui->rotateRight->setTriggerMode(TriggerMode::Press);
    ui->flipH->setAction(u"flipH"_s);
    ui->flipH->setIconPath(u":/res/icons/common/menuitem/flip-h16.png"_s);
    ui->flipH->setTriggerMode(TriggerMode::Press);
    ui->flipV->setAction(u"flipV"_s);
    ui->flipV->setIconPath(u":/res/icons/common/menuitem/flip-v16.png"_s);
    ui->flipV->setTriggerMode(TriggerMode::Press);
    ui->crop->setAction(u"crop"_s);
    ui->crop->setIconPath(u":/res/icons/common/menuitem/image-crop16.png"_s);
    ui->crop->setTriggerMode(TriggerMode::Press);
    ui->resize->setAction(u"resize"_s);
    ui->resize->setIconPath(u":/res/icons/common/menuitem/resize16.png"_s);
    ui->resize->setTriggerMode(TriggerMode::Press);
    // -------------------------------------------------------------------------
    //  entries
    ui->print->setAction(u"print"_s);
    ui->print->setText(tr("Print"));
    ui->print->setIconPath(u":/res/icons/common/menuitem/print16.png"_s);

    ui->copy->setAction(u"copyFile"_s);
    ui->copy->setText(tr("Quick copy"));
    ui->copy->setIconPath(u":/res/icons/common/menuitem/copy16.png"_s);

    ui->move->setAction(u"moveFile"_s);
    ui->move->setText(tr("Quick move"));
    ui->move->setIconPath(u":/res/icons/common/menuitem/move16.png"_s);

    ui->trash->setAction(u"moveToTrash"_s);
    ui->trash->setText(tr("Move to trash"));
    ui->trash->setIconPath(u":/res/icons/common/menuitem/trash16.png"_s);
    ui->trash->setShortcutText(u""_s);
    // -------------------------------------------------------------------------
    ui->open->setAction(u"open"_s);
    ui->open->setText(tr("Open"));
    ui->open->setIconPath(u":/res/icons/common/menuitem/open16.png"_s);
    ui->open->setShortcutText(u""_s);

    ui->folderView->setAction(u"folderView"_s);
    ui->folderView->setText(tr("Folder View"));
    ui->folderView->setIconPath(u":/res/icons/common/menuitem/folderview16.png"_s);
    ui->folderView->setShortcutText(u""_s);

    ui->settings->setAction(u"openSettings"_s);
    ui->settings->setText(tr("Settings"));
    ui->settings->setIconPath(u":/res/icons/common/menuitem/settings16.png"_s);
    // -------------------------------------------------------------------------
    ui->openWith->setText(tr("Open with..."));
    ui->openWith->setIconPath(u":/res/icons/common/menuitem/run16.png"_s);
    ui->openWith->setPassthroughClicks(false);
    connect(ui->openWith, &ContextMenuItem::pressed, this, &ContextMenu::switchToScriptsPage);
    // -------------------------------------------------------------------------
    ui->showLocation->setAction(u"showInDirectory"_s);
    ui->showLocation->setText(tr("Show in folder"));
    ui->showLocation->setIconPath(u":/res/icons/common/menuitem/folder16.png"_s);
    // -------------------------------------------------------------------------
    // force resize to fit new menuitem width
    adjustSize();

    // Scripts page
    // -------------------------------------------------------------------------
    ui->backButton->setText(tr("Back"));
    ui->backButton->setIconPath(u":/res/icons/common/menuitem/back16.png"_s);
    ui->backButton->setPassthroughClicks(false);
    ui->scriptSetupButton->setText(tr("Configure menu"));
    ui->scriptSetupButton->setIconPath(u":/res/icons/common/menuitem/settings16.png"_s);
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
    for (auto const [key, val] : scripts.asKeyValueRange()) {
        if (!val.command.isEmpty()) {
            auto btn = new ContextMenuItem();
            btn->setAction(u"s:"_s + key);
            btn->setIconPath(u":/res/icons/common/menuitem/open16.png"_s);
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
    switch (event->key()) {
    case Qt::Key_Escape:
        hide();
        break;
    case Qt::Key_Up:
    case Qt::Key_Down:
        // TODO: keyboard navigation
        break;
    default:
        actionManager->processEvent(event);
        break;
    }
}
