#include "fullscreeninfooverlayproxy.h"

FullscreenInfoOverlayProxy::FullscreenInfoOverlayProxy(FloatingWidgetContainer *parent)
    : container(parent),
      infoOverlay(nullptr)
{
}

FullscreenInfoOverlayProxy::~FullscreenInfoOverlayProxy()
#if 0
{
    if (infoOverlay)
        infoOverlay->deleteLater();
}
#else
    = default;
#endif

void FullscreenInfoOverlayProxy::show()
{
    stateBuf.showImmediately = true;
    init();
}

void FullscreenInfoOverlayProxy::showWhenReady()
{
    if (!infoOverlay)
        stateBuf.showImmediately = true;
    else
        infoOverlay->show();
}

void FullscreenInfoOverlayProxy::hide()
{
    stateBuf.showImmediately = false;
    if (infoOverlay)
        infoOverlay->hide();
}

void FullscreenInfoOverlayProxy::setInfo(QString const &position, QString const &fileName, QString const &info)
{
    if (infoOverlay) {
        infoOverlay->setInfo(position, fileName, info);
    } else {
        stateBuf.position = position;
        stateBuf.fileName = fileName;
        stateBuf.info     = info;
    }
}

void FullscreenInfoOverlayProxy::init()
{
    if (infoOverlay)
        return;
    infoOverlay = new FullscreenInfoOverlay(container);
    if (!stateBuf.fileName.isEmpty())
        setInfo(stateBuf.position, stateBuf.fileName, stateBuf.info);
    if (stateBuf.showImmediately)
        infoOverlay->show();
}
