#include "slidepanel.h"

SlidePanel::SlidePanel(FloatingWidgetContainer *parent)
    : FloatingWidget(parent),
      panelSize(50),
      slideAmount(40),
      fadeEffect(new QGraphicsOpacityEffect(this)),
      mLayout(new QHBoxLayout(this)),
      mWidget(nullptr)
{
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mLayout);

    // fade effect
    setGraphicsEffect(fadeEffect);
    startPosition = geometry().topLeft();
    outCurve.setType(QEasingCurve::OutQuart);

    timeline.setDuration(ANIMATION_DURATION);
    timeline.setEasingCurve(QEasingCurve::Linear);
    timeline.setStartFrame(0);
    timeline.setEndFrame(ANIMATION_DURATION);
    // For some reason 16 feels janky on windows. Linux is fine.
#ifdef Q_OS_WIN32
    timeline.setUpdateInterval(8);
#else
    timeline.setUpdateInterval(16);
#endif

    connect(&timeline, &QTimeLine::frameChanged, this, &SlidePanel::animationUpdate);
    connect(&timeline, &QTimeLine::finished, this, &SlidePanel::onAnimationFinish);

    setAttribute(Qt::WA_NoMousePropagation, true);
    setFocusPolicy(Qt::NoFocus);
    setPosition(PanelPosition::TOP);

    QWidget::hide();
}

SlidePanel::~SlidePanel() = default;

void SlidePanel::hide()
{
    timeline.stop();
    QWidget::hide();
}

void SlidePanel::hideAnimated()
{
    if (layoutManaged())
        hide();
    else if (!isHidden() && timeline.state() != QTimeLine::Running)
        timeline.start();
}

bool SlidePanel::layoutManaged() const
{
    return mLayoutManaged;
}

void SlidePanel::setLayoutManaged(bool mode)
{
    mLayoutManaged = mode;
    if (!mode)
        recalculateGeometry();
}

void SlidePanel::setWidget(QWidget *w)
{
    if (!w)
        return;
    if (hasWidget())
        layout()->removeWidget(mWidget);
    mWidget = w;
    mWidget->setParent(this);
    mLayout->insertWidget(0, mWidget);
}

bool SlidePanel::hasWidget() const
{
    return mWidget != nullptr;
}

void SlidePanel::show()
{
    if (hasWidget()) {
        timeline.stop();
        fadeEffect->setOpacity(panelVisibleOpacity);
        setProperty("pos", startPosition);
        QWidget::show();
        raise();
    } else {
        qDebug() << u"Warning: Trying to show panel containing no widget!";
    }
}

// save current geometry so it is accessible during "pos" animation
void SlidePanel::saveStaticGeometry(QRect geometry)
{
    mStaticGeometry = geometry;
}

QRect SlidePanel::staticGeometry() const
{
    return mStaticGeometry;
}

void SlidePanel::animationUpdate(int frame)
{
    // Calculate local cursor position; correct for the current pos() animation
    QPoint adjustedPos = mapFromGlobal(QCursor::pos()) + this->pos();

    if (triggerRect().contains(adjustedPos, true)) {
        // Cancel the animation if cursor is back at the panel
        timeline.stop();
        fadeEffect->setOpacity(panelVisibleOpacity);
        setProperty("pos", startPosition);
    } else {
        // Apply the animation frame
        qreal  value        = outCurve.valueForProgress(static_cast<qreal>(frame) / ANIMATION_DURATION);
        QPoint newPosOffset = QPoint(static_cast<int>((endPosition.x() - startPosition.x()) * value),
                                     static_cast<int>((endPosition.y() - startPosition.y()) * value));
        setProperty("pos", startPosition + newPosOffset);
        fadeEffect->setOpacity(1 - value);
    }
    qApp->processEvents();
}

void SlidePanel::setAnimationRange(QPoint start, QPoint end)
{
    startPosition = start;
    endPosition   = end;
}

void SlidePanel::onAnimationFinish()
{
    QWidget::hide();
    fadeEffect->setOpacity(panelVisibleOpacity);
    setProperty("pos", startPosition);
}

QRect SlidePanel::triggerRect() const
{
    return mTriggerRect;
}

void SlidePanel::setPosition(PanelPosition p)
{
    if (p == PanelPosition::TOP || p == PanelPosition::BOTTOM)
        mLayout->setDirection(QBoxLayout::LeftToRight);
    else
        mLayout->setDirection(QBoxLayout::BottomToTop);
    mPosition = p;
    recalculateGeometry();
}

PanelPosition SlidePanel::position() const
{
    return mPosition;
}

void SlidePanel::recalculateGeometry()
{
    if (layoutManaged())
        return;
    if (mPosition == PanelPosition::TOP) {
        setAnimationRange(QPoint(0, 0), QPoint(0, 0) - QPoint(0, slideAmount));
        saveStaticGeometry(QRect(QPoint(0, 0), QPoint(containerSize().width() - 1, height() - 1)));
    } else if (mPosition == PanelPosition::BOTTOM) {
        setAnimationRange(QPoint(0, containerSize().height() - height()),
                          QPoint(0, containerSize().height() - height() + slideAmount));
        saveStaticGeometry(
            QRect(QPoint(0, containerSize().height() - height()), QPoint(containerSize().width() - 1, containerSize().height())));
    } else if (mPosition == PanelPosition::LEFT) {
        setAnimationRange(QPoint(0, 0), QPoint(0, 0) - QPoint(slideAmount, 0));
        saveStaticGeometry(QRect(0, 0, width(), containerSize().height()));

    } else { // right
        setAnimationRange(QPoint(containerSize().width() - width(), 0),
                          QPoint(containerSize().width() - width(), 0) + QPoint(slideAmount, 0));
        saveStaticGeometry(QRect(containerSize().width() - width(), 0, containerSize().width(), containerSize().height()));
    }
    setGeometry(staticGeometry());
    updateTriggerRect();
}

void SlidePanel::updateTriggerRect()
{
    mTriggerRect = staticGeometry();
}

void SlidePanel::setOrientation()
{
}
