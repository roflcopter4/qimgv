#include "Common.h"
#include "mainpanel.h"

namespace {
QSharedPointer<ThumbnailStripProxy> global_thumbnailStrip;
}

MainPanel::MainPanel(FloatingWidgetContainer *parent) : SlidePanel(parent)
{
    // buttons stuff
    buttonsWidget = new QWidget();
    buttonsWidget->setAccessibleName(QS("panelButtonsWidget"));
    openButton = new ActionButton(QS("open"), QS(":res/icons/common/buttons/panel/open20.png"), 30, this);
    openButton->setAccessibleName(QS("ButtonSmall"));
    openButton->setTriggerMode(TriggerMode::Press);
    settingsButton = new ActionButton(QS("openSettings"), QS(":res/icons/common/buttons/panel/settings20.png"), 30, this);
    settingsButton->setAccessibleName(QS("ButtonSmall"));
    settingsButton->setTriggerMode(TriggerMode::Press);
    exitButton = new ActionButton(QS("exit"), QS(":res/icons/common/buttons/panel/close16.png"), 30, this);
    exitButton->setAccessibleName(QS("ButtonSmall"));
    exitButton->setTriggerMode(TriggerMode::Press);
    folderViewButton = new ActionButton(QS("folderView"), QS(":res/icons/common/buttons/panel/folderview20.png"), 30, this);
    folderViewButton->setAccessibleName(QS("ButtonSmall"));
    folderViewButton->setTriggerMode(TriggerMode::Press);
    pinButton = new ActionButton(QS(""), QS(":res/icons/common/buttons/panel/pin-panel20.png"), 30, this);
    pinButton->setAccessibleName(QS("ButtonSmall"));
    pinButton->setTriggerMode(TriggerMode::Press);
    pinButton->setCheckable(true);
    connect(pinButton, &ActionButton::toggled, this, &MainPanel::onPinClicked);

    buttonsLayout = new QVBoxLayout();
    buttonsLayout->setDirection(QBoxLayout::BottomToTop);
    buttonsLayout->setSpacing(0);
    buttonsLayout->addWidget(settingsButton);
    buttonsLayout->addWidget(openButton);
    buttonsLayout->addStretch(0);
    buttonsLayout->addWidget(pinButton);
    buttonsLayout->addWidget(folderViewButton);
    buttonsLayout->addWidget(exitButton);

    buttonsWidget->setLayout(buttonsLayout);
    this->layout()->addWidget(buttonsWidget);

    thumbnailStrip = QSharedPointer<ThumbnailStripProxy>(new ThumbnailStripProxy(this));
    global_thumbnailStrip = thumbnailStrip;
    setWidget(thumbnailStrip);

    readSettings();
    // connect(settings, SIGNAL(settingsChanged()), this, SLOT(readSettings()));
}

MainPanel::~MainPanel()
{
}

void MainPanel::onPinClicked()
{
    bool mode = !settings->panelPinned();
    pinButton->setChecked(mode);
    settings->setPanelPinned(mode);
    emit pinned(mode);
}

void MainPanel::setPosition(PanelPosition p)
{
    SlidePanel::setPosition(p);
    switch (p) {
    case PanelPosition::TOP:
        buttonsLayout->setDirection(QBoxLayout::BottomToTop);
        layout()->setContentsMargins(0, 0, 0, 1);
        buttonsLayout->setContentsMargins(4, 0, 0, 0);
        break;
    case PanelPosition::BOTTOM:
        buttonsLayout->setDirection(QBoxLayout::BottomToTop);
        layout()->setContentsMargins(0, 3, 0, 0);
        buttonsLayout->setContentsMargins(4, 0, 0, 0);
        break;
    case PanelPosition::LEFT:
        buttonsLayout->setDirection(QBoxLayout::LeftToRight);
        layout()->setContentsMargins(0, 0, 1, 0);
        buttonsLayout->setContentsMargins(0, 0, 0, 4);
        break;
    case PanelPosition::RIGHT:
        buttonsLayout->setDirection(QBoxLayout::LeftToRight);
        layout()->setContentsMargins(1, 0, 0, 0);
        buttonsLayout->setContentsMargins(0, 0, 0, 4);
        break;
    }
    recalculateGeometry();
}

void MainPanel::setExitButtonEnabled(bool mode)
{
    exitButton->setHidden(!mode);
}

QSharedPointer<ThumbnailStripProxy> MainPanel::getThumbnailStrip()
{
    return thumbnailStrip;
}

void MainPanel::setupThumbnailStrip()
{
    thumbnailStrip->init();
    // adjust size & position
    readSettings();
}

QSize MainPanel::sizeHint() const
{
    if (!thumbnailStrip->isInitialized())
        return {0, 0};
    // item size + spacing + scrollbar + border
    switch (settings->panelPosition()) {
    case PanelPosition::TOP:
        return {width(), thumbnailStrip->itemSize().height() + 16};
    case PanelPosition::BOTTOM:
        return {width(), thumbnailStrip->itemSize().height() + 16 + 3};
    case PanelPosition::LEFT:
    case PanelPosition::RIGHT:
        return {thumbnailStrip->itemSize().width() + 16, height()};
    }
    return {0, 0};
}

void MainPanel::readSettings()
{
    auto newPos = settings->panelPosition();
    if (newPos == PanelPosition::TOP || newPos == PanelPosition::BOTTOM) {
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        int h = sizeHint().height();
        if (h)
            setFixedHeight(h);
        setFixedWidth(QWIDGETSIZE_MAX);
    } else {
        this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        int w = sizeHint().width();
        if (w)
            setFixedWidth(w);
        setFixedHeight(QWIDGETSIZE_MAX);
    }
    thumbnailStrip->readSettings();
    setPosition(newPos);
    pinButton->setChecked(settings->panelPinned());
}

// draw separator line at bottom or top
void MainPanel::paintEvent(QPaintEvent *event)
{
    FloatingWidget::paintEvent(event);
    // borders
    QPainter p(this);
    p.setPen(settings->colorScheme().folderview_hc);
    switch (mPosition) {
    case PanelPosition::TOP:
        p.drawLine(rect().bottomLeft(), rect().bottomRight());
        break;
    case PanelPosition::BOTTOM:
        p.fillRect(rect().left(), rect().top(), width(), 3, settings->colorScheme().folderview);
        p.drawLine(rect().topLeft(), rect().topRight());
        break;
    case PanelPosition::LEFT:
        p.drawLine(rect().topRight(), rect().bottomRight());
        break;
    case PanelPosition::RIGHT:
        p.drawLine(rect().topLeft(), rect().bottomLeft());
        break;
    }
}
