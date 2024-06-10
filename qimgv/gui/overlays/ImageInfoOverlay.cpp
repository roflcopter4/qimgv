#include "ImageInfoOverlay.h"
#include "ui_ImageInfoOverlay.h"

ImageInfoOverlay::ImageInfoOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::ImageInfoOverlay)
{
    ui->setupUi(this);
    ui->closeButton->setIconPath(QS(":res/icons/common/overlay/close-dim16.png"));
    ui->headerIcon->setIconPath(QS(":res/icons/common/overlay/info16.png"));
    entryStub.setFixedSize(280, 48);
    entryStub.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    connect(ui->closeButton, &IconButton::clicked, this, &ImageInfoOverlay::hide);
    this->setPosition(FloatingWidgetPosition::RIGHT);

    if (parent)
        setContainerSize(parent->size());
}

ImageInfoOverlay::~ImageInfoOverlay()
{
    delete ui;
    for (auto i = entries.count() - 1; i >= 0; i--)
        delete entries.takeAt(i);
}

void ImageInfoOverlay::setExifInfo(QMap<QString, QString> const &info)
{
    // remove/add entries
    qsizetype entryCount = entries.count();
    if (entryCount > info.count()) {
        for (auto i = entryCount - 1; i >= info.count(); i--) {
            ui->entryLayout->removeWidget(entries.last());
            delete entries.takeLast();
        }
    } else if (entryCount < info.count()) {
        for (auto i = entryCount; i < info.count(); i++) {
            entries.append(new EntryInfoItem(this));
            ui->entryLayout->addWidget(entries.last());
        }
    }

    qsizetype entryIdx = 0;
    for (auto i = info.constBegin(); i != info.constEnd(); ++i, ++entryIdx)
        entries[entryIdx]->setInfo(i.key(), i.value());

    // Hiding/showing entryStub causes flicker,
    // so we just remove it from layout and clear the text.
    // It's still there but basically not visible
    if (entries.count()) {
        ui->entryLayout->removeWidget(&entryStub);
        entryStub.setText(QS(""));
    } else {
        ui->entryLayout->addWidget(&entryStub);
        entryStub.setText(QS("<no metadata found>"));
    }

    if (!isHidden() && entryCount != info.count()) {
        // wait for layout change
        qApp->processEvents();
        // reposition
        recalculateGeometry();
    }
}

void ImageInfoOverlay::show()
{
    OverlayWidget::show();
    adjustSize();
    recalculateGeometry();
}

void ImageInfoOverlay::wheelEvent(QWheelEvent *event)
{
    event->accept();
}
