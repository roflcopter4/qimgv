#include "ControlsOverlay.h"

ControlsOverlay::ControlsOverlay(FloatingWidgetContainer *parent)
    : FloatingWidget(parent),
      layout(new QHBoxLayout(this))
{
    folderViewButton = new ActionButton(QS("folderView"), QS(":/res/icons/common/buttons/panel/folderview20.png"), 30);
    folderViewButton->setAccessibleName(QS("ButtonSmall"));
    settingsButton = new ActionButton(QS("openSettings"), QS(":/res/icons/common/buttons/panel/settings20.png"), 30);
    settingsButton->setAccessibleName(QS("ButtonSmall"));
    closeButton = new ActionButton(QS("exit"), QS(":/res/icons/common/buttons/panel/close16.png"), 30);
    closeButton->setAccessibleName(QS("ButtonSmall"));

    auto horizontalLineWidget = new QWidget(this);
    horizontalLineWidget->setFixedSize(5, 22);
    horizontalLineWidget->setStyleSheet(QString(QS("background-color: #707070; margin-left: 2px; margin-right: 2px")));

    layout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(folderViewButton);
    layout->addWidget(horizontalLineWidget);
    layout->addWidget(settingsButton);
    layout->addWidget(closeButton);
    setLayout(layout);
    fitToContents();

    setMouseTracking(true);

    fadeEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(fadeEffect);
    fadeAnimation = new QPropertyAnimation(fadeEffect, "opacity", this);
    fadeAnimation->setDuration(230);
    fadeAnimation->setStartValue(1.0f);
    fadeAnimation->setEndValue(0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutQuart);

    if (parent)
        setContainerSize(parent->size());
    // show();
}

void ControlsOverlay::show()
{
    fadeEffect->setOpacity(0.0);
    FloatingWidget::show();
}

QSize ControlsOverlay::contentsSize() const
{
    QSize newSize(0, 0);
    for (int i = 0; i < layout->count(); i++) {
        newSize.setWidth(newSize.width() + layout->itemAt(i)->widget()->width());
        newSize.setHeight(layout->itemAt(i)->widget()->height());
    }
    return newSize;
}

void ControlsOverlay::fitToContents()
{
    setFixedSize(contentsSize());
    recalculateGeometry();
}

void ControlsOverlay::recalculateGeometry()
{
    setGeometry(containerSize().width() - width(), 0, width(), height());
}

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
void ControlsOverlay::enterEvent(QEnterEvent *event)
{
#else
void ControlsOverlay::enterEvent(QEvent *event)
{
#endif
    Q_UNUSED(event)
    fadeAnimation->stop();
    fadeEffect->setOpacity(1.0);
}

void ControlsOverlay::leaveEvent(QEvent *event)
{
    fadeAnimation->start();
}
