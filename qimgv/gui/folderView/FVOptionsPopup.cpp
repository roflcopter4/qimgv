#include "FVOptionsPopup.h"
#include "ui_FVOptionsPopup.h"

FVOptionsPopup::FVOptionsPopup(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::FVOptionsPopup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground, true);

    ui->viewSimpleButton->  setText(QObject::tr("Simple"));
    ui->viewExtendedButton->setText(QObject::tr("Extended"));
    ui->viewFoldersButton-> setText(QObject::tr("Extended + Folders"));

    connect(ui->viewSimpleButton,   &ContextMenuItem::pressed, this, &FVOptionsPopup::selectSimpleView);
    connect(ui->viewExtendedButton, &ContextMenuItem::pressed, this, &FVOptionsPopup::selectExtendedView);
    connect(ui->viewFoldersButton,  &ContextMenuItem::pressed, this, &FVOptionsPopup::selectFoldersView);

    // Force size recalculation
    adjustSize();
    readSettings();
    connect(settings, &Settings::settingsChanged, this, &FVOptionsPopup::readSettings);
    hide();
}

FVOptionsPopup::~FVOptionsPopup()
{
    delete ui;
}

void FVOptionsPopup::setSimpleView()
{
    ui->viewSimpleButton->setIconPath(u":res/icons/common/buttons/panel-small/add-new12.png"_s);
    ui->viewExtendedButton->setIconPath(u""_s);
    ui->viewFoldersButton->setIconPath(u""_s);
}

void FVOptionsPopup::setExtendedView()
{
    ui->viewSimpleButton->setIconPath(u""_s);
    ui->viewExtendedButton->setIconPath(u":res/icons/common/buttons/panel-small/add-new12.png"_s);
    ui->viewFoldersButton->setIconPath(u""_s);
}

void FVOptionsPopup::setFoldersView()
{
    ui->viewSimpleButton->setIconPath(u""_s);
    ui->viewExtendedButton->setIconPath(u""_s);
    ui->viewFoldersButton->setIconPath(u":res/icons/common/buttons/panel-small/add-new12.png"_s);
}

void FVOptionsPopup::selectSimpleView()
{
    setSimpleView();
    emit viewModeSelected(FolderViewMode::SIMPLE);
}

void FVOptionsPopup::selectExtendedView()
{
    setExtendedView();
    emit viewModeSelected(FolderViewMode::EXTENDED);
}

void FVOptionsPopup::selectFoldersView()
{
    setFoldersView();
    emit viewModeSelected(FolderViewMode::EXT_FOLDERS);
}

void FVOptionsPopup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void FVOptionsPopup::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        hide();
    else
        actionManager->processEvent(event);
}

void FVOptionsPopup::setViewMode(FolderViewMode mode)
{
    if (mode == FolderViewMode::SIMPLE)
        setSimpleView();
    else if (mode == FolderViewMode::EXTENDED)
        setExtendedView();
    else
        setFoldersView();
}

void FVOptionsPopup::readSettings()
{
    setViewMode(settings->folderViewMode());
}

void FVOptionsPopup::showAt(QPoint pos)
{
    QRect geom = geometry();
    geom.moveTopLeft(pos);
    setGeometry(geom);
    show();
}

void FVOptionsPopup::hideEvent(QHideEvent *event)
{
    event->accept();
    emit dismissed();
}
