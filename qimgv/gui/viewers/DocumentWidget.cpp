#include "DocumentWidget.h"
#include <memory>

DocumentWidget::DocumentWidget(ViewerWidget *viewWidget, InfoBarProxy *infoBar, QWidget *parent)
    : FloatingWidgetContainer(parent),
      layout(new QBoxLayout(QBoxLayout::LeftToRight)),
      layoutRoot(new QVBoxLayout()),
      mViewWidget(viewWidget),
      mInfoBar(infoBar),
      mainPanel(new MainPanel(this))
{
    layoutRoot->setContentsMargins(0, 0, 0, 0);
    layoutRoot->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layoutRoot->addLayout(layout);
    setLayout(layoutRoot);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    layout->addWidget(mViewWidget);
    mViewWidget->show();

    layoutRoot->addWidget(mInfoBar);
    setFocusProxy(mViewWidget);

    setInteractionEnabled(true);

    connect(mainPanel, &MainPanel::pinned,         this, &DocumentWidget::setPanelPinned);
    connect(settings,  &Settings::settingsChanged, this, &DocumentWidget::readSettings);
    readSettings();
}

DocumentWidget::~DocumentWidget() = default;

ViewerWidget *DocumentWidget::viewWidget() const
{
    return mViewWidget;
}

void DocumentWidget::readSettings()
{
    setPanelEnabled(settings->panelEnabled());
    mPanelFullscreenOnly = settings->panelFullscreenOnly();
    setPanelPinned(settings->panelPinned());
    mainPanel->readSettings();
}

void DocumentWidget::onFullscreenModeChanged(bool mode)
{
    mainPanel->setExitButtonEnabled(settings->panelPosition() == PanelPosition::TOP ||
                                    settings->panelPosition() == PanelPosition::RIGHT
                                        ? mode : false);
    mIsFullscreen = mode;
}

void DocumentWidget::setPanelPinned(bool mode)
{
    if (!mPanelEnabled)
        return;
    if (!mode) { // unpin
        if (mPanelPinned)
            layout->removeWidget(mainPanel);
        mainPanel->setLayoutManaged(false);
    } else { // pin
        layout->insertWidget(1, mainPanel);
        switch (settings->panelPosition()) {
        case PanelPosition::TOP:
            layout->setDirection(QBoxLayout::BottomToTop);
            break;
        case PanelPosition::BOTTOM:
            layout->setDirection(QBoxLayout::TopToBottom);
            break;
        case PanelPosition::LEFT:
            layout->setDirection(QBoxLayout::RightToLeft);
            break;
        case PanelPosition::RIGHT:
            layout->setDirection(QBoxLayout::LeftToRight);
            break;
        }
        mainPanel->setLayoutManaged(true);
        mainPanel->show();
    }
    mPanelPinned = mode;
}

bool DocumentWidget::panelPinned() const
{
    return mPanelPinned;
}

ThumbnailStripProxy *DocumentWidget::thumbPanel() const
{
    return mainPanel->getThumbnailStrip();
}

void DocumentWidget::hideFloatingPanel()
{
    hideFloatingPanel(false);
}

void DocumentWidget::hideFloatingPanel(bool)
{
    if (!mPanelPinned)
        mainPanel->hideAnimated();
}

void DocumentWidget::setPanelEnabled(bool mode)
{
    mPanelEnabled = mode;
    if (!mode)
        mainPanel->hide();
    else
        setupMainPanel();
}

bool DocumentWidget::panelEnabled() const
{
    return mPanelEnabled;
}

void DocumentWidget::allowPanelInit()
{
    mAllowPanelInit = true;
}

void DocumentWidget::setupMainPanel()
{
    if (mPanelEnabled && mAllowPanelInit)
        mainPanel->setupThumbnailStrip();
}

void DocumentWidget::setInteractionEnabled(bool mode)
{
    mInteractionEnabled = mode;
    if (!mode && !mPanelPinned)
        mainPanel->hide();
}

void DocumentWidget::mouseMoveEvent(QMouseEvent *event)
{
    event->ignore();
    if (mPanelPinned)
        return;
    // ignore if we are doing something with the mouse (zoom / drag)
    if (event->buttons() != Qt::NoButton) {
        if (mainPanel->triggerRect().contains(event->pos()))
            mAvoidPanelFlag = true;
        return;
    }
    // show on hover event
    if (mPanelEnabled && (mIsFullscreen || !mPanelFullscreenOnly) &&
        (mainPanel->triggerRect().contains(event->pos()) && !mAvoidPanelFlag))
    {
        mainPanel->show();
    }
    // fade out on leave event
    if (!mainPanel->isHidden()) {
        // leaveEvent which misfires on HiDPI (rounding error somewhere?)
        // add a few px of buffer area to avoid bugs
        // it still fcks up Fitts law as the buttons are not receiving hover on screen border

        // alright this also only works when in root window. sad.
        if (!mainPanel->triggerRect().adjusted(-8, -8, 8, 8).contains(event->pos()))
            mainPanel->hideAnimated();
    }
    if (!mainPanel->triggerRect().contains(event->pos()))
        mAvoidPanelFlag = false;
}

void DocumentWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    if (!mInteractionEnabled)
        return;
    // we can't track move events outside the window (without additional timer),
    // so just hook the panel event here
    if (!mPanelPinned && mPanelEnabled && (mIsFullscreen || !mPanelFullscreenOnly) &&
        (mainPanel->triggerRect().contains(mapFromGlobal(QCursor::pos())) && !mAvoidPanelFlag))
    {
        mainPanel->show();
    }
}

void DocumentWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    // qDebug() << cursor().pos() << rect();
    //  this misfires on hidpi.
    // instead do the panel hiding in MW::leaveEvent  (it works properly in root window)
    // mainPanel->hide();
    mAvoidPanelFlag = false;
}