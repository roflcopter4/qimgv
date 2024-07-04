#pragma once

#include "gui/overlays/FullscreenInfoOverlay.h"

class FullscreenInfoOverlayProxy
{
    struct StateBuffer {
        QString position;
        QString fileName;
        QString info;
        bool    showImmediately = false;
    };

  public:
    explicit FullscreenInfoOverlayProxy(FloatingWidgetContainer *parent = nullptr);
    ~FullscreenInfoOverlayProxy();
    DELETE_COPY_MOVE_ROUTINES(FullscreenInfoOverlayProxy);

    void init();
    void show();
    void showWhenReady();
    void hide();
    void setInfo(QString const &position, QString const &fileName, QString const &info);

  private:
    FloatingWidgetContainer *container;
    FullscreenInfoOverlay   *infoOverlay;
    StateBuffer stateBuf;
};
