#pragma once

#include "gui/overlays/ImageInfoOverlay.h"

class ImageInfoOverlayProxy
{
  public:
    explicit ImageInfoOverlayProxy(FloatingWidgetContainer *parent = nullptr);
    ~ImageInfoOverlayProxy();

    void init();
    void show();
    void hide();

    void setExifInfo(QMap<QString, QString> const &xinfo);
    ND bool isHidden() const;

  private:
    FloatingWidgetContainer *container;
    ImageInfoOverlay        *overlay;
    QMap<QString, QString>   stateBuf;
};
