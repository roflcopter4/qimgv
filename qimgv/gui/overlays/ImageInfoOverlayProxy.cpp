#include "ImageInfoOverlayProxy.h"

ImageInfoOverlayProxy::ImageInfoOverlayProxy(FloatingWidgetContainer *parent)
    : container(parent),
      overlay(nullptr)
{}

ImageInfoOverlayProxy::~ImageInfoOverlayProxy()
#if 0
{
    if (overlay)
        overlay->deleteLater();
}
#else
    = default;
#endif



void ImageInfoOverlayProxy::show()
{
    init();
    overlay->show();
}

void ImageInfoOverlayProxy::hide()
{
    if (overlay)
        overlay->hide();
}

void ImageInfoOverlayProxy::init()
{
    if (overlay)
        return;
    overlay = new ImageInfoOverlay(container);
    overlay->setExifInfo(stateBuf);
}

bool ImageInfoOverlayProxy::isHidden() const
{
    return overlay ? overlay->isHidden() : true;
}

void ImageInfoOverlayProxy::setExifInfo(QMap<QString, QString> const &xinfo)
{
    if (overlay)
        overlay->setExifInfo(xinfo);
    else
        stateBuf = xinfo;
}
