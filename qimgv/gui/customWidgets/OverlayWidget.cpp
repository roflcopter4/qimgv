#include "OverlayWidget.h"

OverlayWidget::OverlayWidget(FloatingWidgetContainer *parent)
    : FloatingWidget(parent),
      opacityEffect(new QGraphicsOpacityEffect(this)),
      fadeAnimation(new QPropertyAnimation(this, "opacity"_ba, this))
{
    opacityEffect->setOpacity(1.0);
    setGraphicsEffect(opacityEffect);
    fadeAnimation->setDuration(120);
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutQuad);

    connect(fadeAnimation, &QPropertyAnimation::finished, this, &QWidget::hide);
}

OverlayWidget::~OverlayWidget() = default;

qreal OverlayWidget::opacity() const
{
    return opacityEffect->opacity();
}

void OverlayWidget::setOpacity(qreal opacity)
{
    opacityEffect->setOpacity(opacity);
    update();
}

void OverlayWidget::setHorizontalMargin(int margin)
{
    mHorizontalMargin = margin;
    recalculateGeometry();
}

void OverlayWidget::setVerticalMargin(int margin)
{
    mVerticalMargin = margin;
    recalculateGeometry();
}

int OverlayWidget::horizontalMargin() const
{
    return mHorizontalMargin;
}

int OverlayWidget::verticalMargin() const
{
    return mVerticalMargin;
}

void OverlayWidget::setPosition(FloatingWidget::Position pos)
{
    position = pos;
    recalculateGeometry();
}

void OverlayWidget::setFadeDuration(int duration)
{
    fadeAnimation->setDuration(duration);
}

void OverlayWidget::setFadeEnabled(bool mode)
{
    fadeEnabled = mode;
}

void OverlayWidget::show()
{
    fadeAnimation->stop();
    opacityEffect->setOpacity(1.0);
    FloatingWidget::show();
}

void OverlayWidget::hideAnimated()
{
    if (fadeEnabled && !isHidden()) {
        fadeAnimation->stop();
        fadeAnimation->start(QPropertyAnimation::KeepWhenStopped);
    } else {
        FloatingWidget::hide();
    }
}

void OverlayWidget::hide()
{
    fadeAnimation->stop();
    FloatingWidget::hide();
}

void OverlayWidget::recalculateGeometry()
{
    QRect  newRect = QRect(QPoint(0, 0), sizeHint());
    QPoint pos(0, 0);

    switch (position) {
    case FloatingWidget::Position::Left:
        pos.setX(mHorizontalMargin);
        pos.setY((containerSize().height() - newRect.height()) / 2);
        break;
    case FloatingWidget::Position::Right:
        pos.setX(containerSize().width() - newRect.width() - mHorizontalMargin);
        pos.setY((containerSize().height() - newRect.height()) / 2);
        break;
    case FloatingWidget::Position::Bottom:
        pos.setX((containerSize().width() - newRect.width()) / 2);
        pos.setY(containerSize().height() - newRect.height() - mVerticalMargin);
        break;
    case FloatingWidget::Position::Top:
        pos.setX((containerSize().width() - newRect.width()) / 2);
        pos.setY(mVerticalMargin);
        break;
    case FloatingWidget::Position::TopLeft:
        pos.setX(mHorizontalMargin);
        pos.setY(mVerticalMargin);
        break;
    case FloatingWidget::Position::TopRight:
        pos.setX(containerSize().width() - newRect.width() - mHorizontalMargin);
        pos.setY(mVerticalMargin);
        break;
    case FloatingWidget::Position::BottomLeft:
        pos.setX(mHorizontalMargin);
        pos.setY(containerSize().height() - newRect.height() - mVerticalMargin);
        break;
    case FloatingWidget::Position::BottomRight:
        pos.setX(containerSize().width() - newRect.width() - mHorizontalMargin);
        pos.setY(containerSize().height() - newRect.height() - mVerticalMargin);
        break;
    case FloatingWidget::Position::Center:
        pos.setX((containerSize().width() - newRect.width()) / 2);
        pos.setY((containerSize().height() - newRect.height()) / 2);
        break;
    }

    // apply position
    newRect.moveTopLeft(pos);
    setGeometry(newRect);
}
