#include "SaveConfirmOverlay.h"
#include "ui_SaveConfirmOverlay.h"

SaveConfirmOverlay::SaveConfirmOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::SaveConfirmOverlay)
{
    ui->setupUi(this);
    connect(ui->saveButton, &QPushButton::clicked, this, &SaveConfirmOverlay::saveClicked);
    connect(ui->saveAsButton, &QPushButton::clicked, this, &SaveConfirmOverlay::saveAsClicked);
    connect(ui->discardButton, &QPushButton::clicked, this, &SaveConfirmOverlay::discardClicked);
    setFocusPolicy(Qt::NoFocus);
    ui->closeButton->setIconPath(u":res/icons/common/overlay/close-dim16.png"_s);
    ui->headerIcon->setIconPath(u":res/icons/common/overlay/edit16.png"_s);
    readSettings();
    connect(settings, &Settings::settingsChanged, this, &SaveConfirmOverlay::readSettings);

    if (parent)
        setContainerSize(parent->size());

    hide();
}

void SaveConfirmOverlay::readSettings()
{
    // don't interfere with the main panel
    if (settings->panelEnabled() && settings->panelPosition() == PanelPosition::BOTTOM)
        setPosition(FloatingWidget::Position::TopRight);
    else
        setPosition(FloatingWidget::Position::BottomRight);
    update();
}

SaveConfirmOverlay::~SaveConfirmOverlay()
{
    delete ui;
}
