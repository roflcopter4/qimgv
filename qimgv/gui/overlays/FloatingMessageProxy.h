#pragma once

#include "gui/overlays/FloatingMessage.h"

class FloatingMessageProxy
{
  public:
    explicit FloatingMessageProxy(FloatingWidgetContainer *parent);
    ~FloatingMessageProxy();

    void showMessage(QString const &text, FloatingMessageIcon icon, int duration);
    void showMessage(QString const &text, FloatingWidgetPosition position, FloatingMessageIcon icon, int duration);
    void init();

  private:
    FloatingWidgetContainer *container;
    FloatingMessage         *overlay;
};
