#pragma once

#include "gui/overlays/ZoomIndicatorOverlay.h"

struct ZoomIndicatorOverlayStateBuffer {
    qreal scale;
};

class ZoomIndicatorOverlayProxy
{
  public:
    explicit ZoomIndicatorOverlayProxy(FloatingWidgetContainer *parent = nullptr);
    ~ZoomIndicatorOverlayProxy();
    DELETE_COPY_ROUTINES(ZoomIndicatorOverlayProxy);
    DEFAULT_MOVE_ROUTINES(ZoomIndicatorOverlayProxy);

    void init();
    void show();
    void show(int duration);
    void hide();
    void setScale(qreal scale);

  private:
    FloatingWidgetContainer *container;
    ZoomIndicatorOverlay    *overlay;
    qreal mScale;
};
