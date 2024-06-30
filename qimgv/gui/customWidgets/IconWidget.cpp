#include "IconWidget.h"

IconWidget::IconWidget(QWidget *parent)
    : QWidget(parent),
      pixmap(nullptr),
      color(settings->colorScheme().icons),
      dpr(devicePixelRatioF())
{
    connect(settings, &Settings::settingsChanged, this, &IconWidget::onSettingsChanged);
}

IconWidget::~IconWidget()
{
    delete pixmap;
}

void IconWidget::setIconPath(QString const &path)
{
    if (iconPath == path)
        return;
    iconPath = path;
    loadIcon();
}

void IconWidget::loadIcon()
{
    auto path = iconPath;
    delete pixmap;
    pixmap = nullptr;
    if (dpr >= 1.0 + 0.001) {
        path.replace(u'.', u"@2x."_s);
        hiResPixmap = true;
        pixmap      = new QPixmap(path);
        if (dpr >= 2.0 - 0.001)
            pixmapDrawScale = dpr;
        else
            pixmapDrawScale = 2.0;
        pixmap->setDevicePixelRatio(pixmapDrawScale);
    } else {
        hiResPixmap     = false;
        pixmap          = new QPixmap(path);
        pixmapDrawScale = dpr;
    }
    applyColor();
    if (pixmap && pixmap->isNull()) {
        delete pixmap;
        pixmap = nullptr;
    }
    update();
}

QSize IconWidget::minimumSizeHint() const
{
    return pixmap && !pixmap->isNull()
        ? pixmap->size() / dpr
        : QWidget::minimumSizeHint();
}

void IconWidget::setIconOffset(int x, int y)
{
    iconOffset.setX(x);
    iconOffset.setY(y);
    update();
}

void IconWidget::setColorMode(IconColorMode _mode)
{
    if (colorMode != _mode && _mode == IconColorMode::SOURCE) {
        colorMode = _mode;
        // reload uncolored
        loadIcon();
    } else {
        colorMode = _mode;
        applyColor();
    }
}

void IconWidget::setColor(QColor const &_color)
{
    colorMode = IconColorMode::CUSTOM;
    color     = _color;
    applyColor();
}

void IconWidget::applyColor()
{
    if (!pixmap || pixmap->isNull() || colorMode == IconColorMode::SOURCE)
        return;
    ImageLib::recolor(*pixmap, color);
}

void IconWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    if (!isEnabled())
        p.setOpacity(0.5);
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    if (pixmap) {
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        QPointF pos;
        if (hiResPixmap) {
            pos = QPointF(static_cast<qreal>(width()) / 2.0 - static_cast<qreal>(pixmap->width()) / (2.0 * pixmapDrawScale),
                          static_cast<qreal>(height()) / 2.0 - static_cast<qreal>(pixmap->height()) / (2.0 * pixmapDrawScale));
        } else {
            pos = QPointF(static_cast<qreal>(width()) / 2.0 - static_cast<qreal>(pixmap->width()) / 2.0,
                          static_cast<qreal>(height()) / 2.0 - static_cast<qreal>(pixmap->height()) / 2.0);
        }
        p.drawPixmap(pos + iconOffset, *pixmap);
    }
}

void IconWidget::onSettingsChanged()
{
    if (colorMode == IconColorMode::THEME && color != settings->colorScheme().icons) {
        color = settings->colorScheme().icons;
        applyColor();
    }
}
