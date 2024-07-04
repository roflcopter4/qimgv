#include "ZoomIndicatorOverlay.h"

ZoomIndicatorOverlay::ZoomIndicatorOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      fm(new QFontMetrics(QApplication::font())),
      layout(new QHBoxLayout(this))
{
    visibilityTimer.setSingleShot(true);
    visibilityTimer.setInterval(hideDelay);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&label);
    label.setAlignment(Qt::AlignCenter);

    setLayout(layout);
    setPosition(FloatingWidget::Position::BottomLeft);
    setHorizontalMargin(0);
    setVerticalMargin(16);

    setFadeEnabled(true);
    setFadeDuration(300);

    connect(&visibilityTimer, &QTimer::timeout, this, &ZoomIndicatorOverlay::hideAnimated);

    if (parent)
        setContainerSize(parent->size());
}

void ZoomIndicatorOverlay::setScale(qreal scale)
{
    label.setText(QString::number(qRound(scale * 100.0)) + u'%');
    label.setFixedSize(fm->horizontalAdvance(label.text()) + 14, fm->height() + 12);
    recalculateGeometry();
}

void ZoomIndicatorOverlay::recalculateGeometry()
{
    OverlayWidget::recalculateGeometry();
}

void ZoomIndicatorOverlay::show()
{
    OverlayWidget::show();
}

// "blink" the widget; show then fade out immediately
void ZoomIndicatorOverlay::show(int duration)
{
    visibilityTimer.stop();
    OverlayWidget::show();
    // fade out after delay
    visibilityTimer.setInterval(duration);
    visibilityTimer.start();
}
