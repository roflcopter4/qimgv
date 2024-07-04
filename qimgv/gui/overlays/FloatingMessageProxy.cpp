#include "FloatingMessageProxy.h"

FloatingMessageProxy::FloatingMessageProxy(FloatingWidgetContainer *parent)
    : container(parent),
      overlay(nullptr)
{}

void FloatingMessageProxy::showMessage(QString const &text, FloatingMessage::Icon icon, int duration)
{
    init();
    overlay->showMessage(text, icon, duration);
}

void FloatingMessageProxy::showMessage(
      QString const           &text,
      FloatingWidget::Position position,
      FloatingMessage::Icon    icon,
      int                      duration)
{
    init();
    overlay->showMessage(text, position, icon, duration);
}

void FloatingMessageProxy::init()
{
    if (overlay)
        return;
    overlay = new FloatingMessage(container);
}
