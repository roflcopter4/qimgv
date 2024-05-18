#include "fullscreeninfooverlay.h"
#include "ui_fullscreeninfooverlay.h"

#include "Common.h"

FullscreenInfoOverlay::FullscreenInfoOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::FullscreenInfoOverlay)
{
    ui->setupUi(this);
    setPosition(FloatingWidgetPosition::TOPLEFT);
    this->setHorizontalMargin(0);
    this->setVerticalMargin(0);
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
    this->adjustSize();
}
