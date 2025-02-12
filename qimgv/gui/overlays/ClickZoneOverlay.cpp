#include "clickzoneoverlay.h"

ClickZoneOverlay::ClickZoneOverlay(FloatingWidgetContainer *parent)
    : FloatingWidget(parent),
      pixmapLeft(loadPixmap(u":/res/icons/common/overlay/arrow_left_50.png"_s)),
      pixmapRight(loadPixmap(u":/res/icons/common/overlay/arrow_right_50.png"_s)),
      dpr(devicePixelRatioF()),
      pixmapDrawScale(dpr)
{
    // this is just for painting, we are handling mouse events elsewhere
    setAttribute(Qt::WA_TransparentForMouseEvents);
    if (parent)
        setContainerSize(parent->size());
    connect(settings, &Settings::settingsChanged, this, &ClickZoneOverlay::readSettings);
    readSettings();
    show();
}

ClickZoneOverlay::~ClickZoneOverlay()
{
    delete pixmapLeft;
    delete pixmapRight;
}

void ClickZoneOverlay::readSettings()
{
    if (settings->clickableEdgesVisible() == drawZones)
        return;
    drawZones = settings->clickableEdgesVisible();
    update();
}

QPixmap *ClickZoneOverlay::loadPixmap(QString path)
{
    QPixmap *pixmap;
    if (dpr >= 1.0 + 0.001) {
        path.replace(u'.', u"@2x."_s);
        pixmap          = new QPixmap(path);
        hiResPixmaps    = true;
        pixmapDrawScale = dpr >= 2.0 - 0.001 ? dpr : 2.0;
        pixmap->setDevicePixelRatio(pixmapDrawScale);
    } else {
        pixmap          = new QPixmap(path);
        hiResPixmaps    = false;
        pixmapDrawScale = dpr;
    }
    ImageLib::recolor(*pixmap, QColor(255, 255, 255));
    if (pixmap->isNull()) {
        delete pixmap;
        pixmap = new QPixmap();
    }
    return pixmap;
}

QRectF ClickZoneOverlay::leftZone() const
{
    return mLeftZone;
}

QRectF ClickZoneOverlay::rightZone() const
{
    return mRightZone;
}

void ClickZoneOverlay::highlightLeft()
{
    setHighlightedZone(ActiveHighlightZone::LEFT);
}

void ClickZoneOverlay::highlightRight()
{
    setHighlightedZone(ActiveHighlightZone::RIGHT);
}

void ClickZoneOverlay::disableHighlight()
{
    setHighlightedZone(ActiveHighlightZone::NONE);
}

bool ClickZoneOverlay::isHighlighted() const
{
    return activeZone != ActiveHighlightZone::NONE;
}

void ClickZoneOverlay::setPressed(bool mode)
{
    if (isPressed == mode)
        return;
    isPressed = mode;
    if (isHighlighted())
        update();
}

void ClickZoneOverlay::setHighlightedZone(ActiveHighlightZone zone)
{
    activeZone = zone;
    update();
}

void ClickZoneOverlay::recalculateGeometry()
{
    setGeometry(0, 0, containerSize().width(), containerSize().height());
}

void ClickZoneOverlay::resizeEvent(QResizeEvent *)
{
    mLeftZone  = QRect(0, 0, zoneSize, height());
    mRightZone = QRect(width() - zoneSize, 0, zoneSize, height());
}

void ClickZoneOverlay::paintEvent(QPaintEvent *)
{
    if (activeZone == ActiveHighlightZone::NONE || !drawZones || width() <= 250)
        return;

    QPainter p(this);
    p.setOpacity(isPressed ? 0.06 : 0.10);
    QBrush brush;
    brush.setColor(QColor(200, 200, 200));
    brush.setStyle(Qt::SolidPattern);

    if (activeZone == ActiveHighlightZone::LEFT) {
        p.fillRect(mLeftZone, brush);
        drawPixmap(p, pixmapLeft, mLeftZone);
    }
    else if (activeZone == ActiveHighlightZone::RIGHT) {
        p.fillRect(mRightZone, brush);
        drawPixmap(p, pixmapRight, mRightZone);
    }
}

// draws pixmap centered inside rect
void ClickZoneOverlay::drawPixmap(QPainter &p, QPixmap const *pixmap, QRectF const &rect) const
{
    p.setOpacity(isPressed ? 0.37 : 0.5);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    float   width  = static_cast<float>(pixmap->width());
    float   height = static_cast<float>(pixmap->height());
    float   scale  = hiResPixmaps ? 2.0f * static_cast<float>(pixmapDrawScale) : 2.0f;
    QPointF pos    = {rect.left() + rect.width() / 2.0f - width / scale,
                      rect.top() + rect.height() / 2.0f - height / scale};

    p.drawPixmap(pos, *pixmap);
}
