#include "FloatingMessageProxy.h"

FloatingMessageProxy::FloatingMessageProxy(FloatingWidgetContainer *parent)
    : container(parent),
      overlay(nullptr)
{}

FloatingMessageProxy::~FloatingMessageProxy() = default;

void FloatingMessageProxy::showMessage(QString const &text, FloatingMessageIcon icon, int duration)
{
    init();
    overlay->showMessage(text, icon, duration);
}

void FloatingMessageProxy::showMessage(
      QString const         &text,
      FloatingWidgetPosition position,
      FloatingMessageIcon    icon,
      int                    duration)
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