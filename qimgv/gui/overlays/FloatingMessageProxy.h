#pragma once

#include "gui/overlays/FloatingMessage.h"

class FloatingMessageProxy
{
  public:
    explicit FloatingMessageProxy(FloatingWidgetContainer *parent);
    ~FloatingMessageProxy() = default;
    DEFAULT_COPY_MOVE_ROUTINES(FloatingMessageProxy);

    void showMessage(QString const &text, FloatingMessage::Icon icon, int duration);
    void showMessage(QString const &text, FloatingWidget::Position position, FloatingMessage::Icon icon, int duration);
    void init();

  private:
    FloatingWidgetContainer *container;
    FloatingMessage         *overlay;
};
