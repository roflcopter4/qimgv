#include "FullscreenInfoOverlay.h"
#include "ui_FullscreenInfoOverlay.h"

FullscreenInfoOverlay::FullscreenInfoOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::FullscreenInfoOverlay)
{
    ui->setupUi(this);
    setPosition(FloatingWidget::Position::TopLeft);
    setHorizontalMargin(0);
    setVerticalMargin(0);
    ui->nameLabel->setText(u"No file opened"_s);
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