#include "SharedResources.h"

// TODO: is there a point in doing this? qt does implicit sharing for pixmaps? test

SharedResources *shrRes = nullptr;

SharedResources::SharedResources() = default;

SharedResources::~SharedResources()
{
    delete mLoadingErrorIcon72;
    delete mLoadingIcon72;
}

QPixmap *SharedResources::getPixmap(ShrIcon icon, qreal dpr)
{
    QPixmap *pixmap;
    QString  path;
    if (icon == ShrIcon::Error) {
        path   = u":/res/icons/common/other/loading-error72.png"_s;
        pixmap = mLoadingErrorIcon72;
    } else {
        path   = u":/res/icons/common/other/loading72.png"_s;
        pixmap = mLoadingIcon72;
    }
    if (pixmap)
        return pixmap;

    if (dpr >= 1.0 + 0.001) {
        path.replace(u'.', u"@2x."_s);
        pixmap = new QPixmap(path);
        qreal pixmapDrawScale = dpr >= 2.0 - 0.001 ? dpr : 2.0;
        pixmap->setDevicePixelRatio(pixmapDrawScale);
    } else {
        pixmap = new QPixmap(path);
    }
    if (icon == ShrIcon::Error)
        mLoadingErrorIcon72 = pixmap;
    else
        mLoadingIcon72 = pixmap;
    return pixmap;
}

SharedResources *SharedResources::getInstance()
{
    if (!shrRes)
        shrRes = new SharedResources();
    return shrRes;
}