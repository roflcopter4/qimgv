#include "FloatingMessage.h"
#include "ui_FloatingMessage.h"

FloatingMessage::FloatingMessage(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::FloatingMessage),
      hideDelay(700),
      preferredPosition(FloatingWidget::Position::Bottom)
{
    ui->setupUi(this);

    visibilityTimer.setSingleShot(true);
    visibilityTimer.setInterval(hideDelay);

    setFadeEnabled(true);
    setFadeDuration(300);
    setIcon(FloatingMessage::Icon::None);

    setAccessibleName(u"FloatingMessage"_s);
    connect(&visibilityTimer, &QTimer::timeout, this, &FloatingMessage::hideAnimated);

    readSettings();

    connect(settings, &Settings::settingsChanged, this, &FloatingMessage::readSettings);

    if (parent)
        setContainerSize(parent->size());
}

FloatingMessage::~FloatingMessage()
{
    delete ui;
}

void FloatingMessage::readSettings()
{
#if 0
    // don't interfere with the main panel
    if (settings->panelEnabled() && settings->panelPosition() == PanelHPosition::PANEL_BOTTOM)
        preferredPosition = FloatingWidget::Position::Top;
    else
        preferredPosition = FloatingWidget::Position::Bottom;
#endif
}

void FloatingMessage::showMessage(QString const &text, FloatingWidget::Position Position, FloatingMessage::Icon icon, int duration)
{
    setPosition(Position);
    doShowMessage(text, icon, duration);
}

void FloatingMessage::showMessage(QString const &text, FloatingMessage::Icon icon, int duration)
{
    setPosition(preferredPosition);
    doShowMessage(text, icon, duration);
}

void FloatingMessage::doShowMessage(QString const &text, FloatingMessage::Icon icon, int duration)
{
    hideDelay = duration;
    setIcon(icon);
    setText(text);
    show();
}

void FloatingMessage::setText(QString const &text)
{
    ui->textLabel->setText(text);
    text.isEmpty() ? ui->textLabel->hide() : ui->textLabel->show();
    recalculateGeometry();
    update();
}

void FloatingMessage::setIcon(FloatingMessage::Icon icon)
{
    switch (icon) {
    case FloatingMessage::Icon::None:
    case FloatingMessage::Icon::Warning:
    case FloatingMessage::Icon::Error:
        // ui->iconLabel->setIconPath(u":/res/icons/common/notifications/error16.png"_s);
        ui->iconLabel->hide();
        break;
    case FloatingMessage::Icon::Directory:
        ui->iconLabel->show();
        ui->iconLabel->setIconPath(u":/res/icons/common/buttons/panel/folder16.png"_s);
        break;
    case FloatingMessage::Icon::LeftEdge:
        ui->iconLabel->show();
        ui->iconLabel->setIconPath(u":/res/icons/common/notifications/dir_start20.png"_s);
        break;
    case FloatingMessage::Icon::RightEdge:
        ui->iconLabel->show();
        ui->iconLabel->setIconPath(u":/res/icons/common/notifications/dir_end20.png"_s);
        break;
    case FloatingMessage::Icon::Success:
        ui->iconLabel->show();
        ui->iconLabel->setIconPath(u":/res/icons/common/notifications/success16.png"_s);
        break;
    }
}

void FloatingMessage::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

// u"blink"_s the widget; show then fade out immediately
void FloatingMessage::show()
{
    visibilityTimer.stop();
    OverlayWidget::show();
    // fade out after delay
    visibilityTimer.setInterval(hideDelay);
    visibilityTimer.start();
}
