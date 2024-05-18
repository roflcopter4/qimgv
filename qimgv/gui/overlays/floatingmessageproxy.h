#pragma once

#include "gui/overlays/floatingmessage.h"

class FloatingMessageProxy
{
public:
    FloatingMessageProxy(FloatingWidgetContainer *parent);
    ~FloatingMessageProxy();
    void showMessage(QString const &text, FloatingMessageIcon icon, int duration);
    void showMessage(QString const &text, FloatingWidgetPosition position, FloatingMessageIcon icon, int duration);
    void init();
private:
    FloatingWidgetContainer *container;
    FloatingMessage *overlay;
};
