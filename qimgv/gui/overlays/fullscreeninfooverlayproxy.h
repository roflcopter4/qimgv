#pragma once

#include "gui/overlays/fullscreeninfooverlay.h"

struct InfoOverlayStateBuffer {
    QString position;
    QString fileName;
    QString info;
    bool showImmediately = false;
};

class FullscreenInfoOverlayProxy {
public:
    explicit FullscreenInfoOverlayProxy(FloatingWidgetContainer *parent = nullptr);
    ~FullscreenInfoOverlayProxy();
    void init();
    void show();
    void showWhenReady();
    void hide();
    void setInfo(QString const &position, QString const &fileName, QString const &info);

private:
    FloatingWidgetContainer *container;
    FullscreenInfoOverlay *infoOverlay;
    InfoOverlayStateBuffer stateBuf;
};
