#include "FloatingWidget.h"
#include <iostream>

FloatingWidget::FloatingWidget(FloatingWidgetContainer *parent) : QWidget(parent), mAcceptKeyboardFocus(false)
{
    setAccessibleName(u"OverlayWidget"_s);
    connect(parent, &FloatingWidgetContainer::resized, this, &FloatingWidget::onContainerResized);
    hide();
}

FloatingWidget::~FloatingWidget()
{
}

QSize FloatingWidget::containerSize() const
{
    return container;
}

void FloatingWidget::setContainerSize(QSize newContainer)
{
    container = newContainer;
    recalculateGeometry();
}

void FloatingWidget::onContainerResized(QSize size)
{
    setContainerSize(size);
}

void FloatingWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

bool FloatingWidget::acceptKeyboardFocus() const
{
    return mAcceptKeyboardFocus;
}
void FloatingWidget::setAcceptKeyboardFocus(bool mode)
{
    mAcceptKeyboardFocus = mode;
}

void FloatingWidget::recalculateGeometry()
{
}

void FloatingWidget::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void FloatingWidget::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}

void FloatingWidget::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void FloatingWidget::hide()
{
    QWidget::hide();
    if (hasFocus() || isAncestorOf(qApp->focusWidget()))
        parentWidget()->setFocus();
}
