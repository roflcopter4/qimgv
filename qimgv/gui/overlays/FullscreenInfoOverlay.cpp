#include "FullscreenInfoOverlay.h"
#include "ui_FullscreenInfoOverlay.h"

FullscreenInfoOverlay::FullscreenInfoOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::FullscreenInfoOverlay)
{
    ui->setupUi(this);
    setPosition(FloatingWidgetPosition::TOPLEFT);
    setHorizontalMargin(0);
    setVerticalMargin(0);
    ui->nameLabel->setText(QS("No file opened"));
    if (parent)
        setContainerSize(parent->size());
}

FullscreenInfoOverlay::~FullscreenInfoOverlay()
{
    delete ui;
}

void FullscreenInfoOverlay::setInfo(QString const &pos, QString const &fileName, QString const &info)
{
    ui->posLabel->setText(pos);
    ui->nameLabel->setText(fileName);
    ui->infoLabel->setText(info);
    adjustSize();
}
