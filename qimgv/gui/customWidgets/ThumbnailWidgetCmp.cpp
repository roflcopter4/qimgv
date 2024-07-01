#include "ThumbnailWidgetCmp.h"

ThumbnailWidgetCmp::ThumbnailWidgetCmp(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    fontName.setBold(false);
    fontInfo.setBold(false);
    QFontMetrics fm(fontName);
    textHeight = fm.height();
    shadowColor.setRgb(0, 0, 0, 60);
}

void ThumbnailWidgetCmp::setThumbnailSize(int size)
{
    if (mThumbnailSize != size && size > 0) {
        isLoaded       = false;
        mThumbnailSize = size;
        updateBoundingRect();
        updateGeometry();
        updateThumbnailDrawPosition();
        updateBackgroundRect();
        setupTextLayout();
        update();
    }
}

void ThumbnailWidgetCmp::setPadding(int newPadding)
{
    padding = newPadding;
    updateBoundingRect();
}

void ThumbnailWidgetCmp::setMargins(int newMarginX, int newMarginY)
{
    marginX = newMarginX;
    marginY = newMarginY;
    updateBoundingRect();
}

int ThumbnailWidgetCmp::thumbnailSize() const
{
    return mThumbnailSize;
}

void ThumbnailWidgetCmp::reset()
{
    if (thumbnail)
        thumbnail.reset();
    highlighted = false;
    hovered     = false;
    isLoaded    = false;
    update();
}

void ThumbnailWidgetCmp::setThumbStyle(ThumbnailStyle style)
{
    if (thumbStyle != style) {
        if (style == ThumbnailStyle::COMPACT) {
            fontName.setBold(true);
            fontInfo.setBold(true);
            if (fontName.pointSizeF() > 14.0) // for some weird setups
                fontInfo.setPointSizeF(fontName.pointSizeF() * 0.85);
            else
                fontInfo.setPointSizeF(fontName.pointSizeF() - 1.0);
        } else {
            fontName.setBold(false);
            fontInfo = fontName;
        }
        thumbStyle = style;
        updateBoundingRect();
        updateThumbnailDrawPosition();
        setupTextLayout();
        updateBackgroundRect();
        updateGeometry();
        update();
    }
}

void ThumbnailWidgetCmp::updateGeometry()
{
    QGraphicsWidget::updateGeometry();
}

void ThumbnailWidgetCmp::setGeometry(QRectF const &rect)
{
    QGraphicsWidget::setGeometry(QRectF(rect.topLeft(), boundingRect().size()));
}

QRectF ThumbnailWidgetCmp::geometry() const
{
    return {QGraphicsWidget::geometry().topLeft(), boundingRect().size()};
}

QSizeF ThumbnailWidgetCmp::effectiveSizeHint(Qt::SizeHint which, QSizeF const &constraint) const
{
    return sizeHint(which, constraint);
}

void ThumbnailWidgetCmp::setThumbnail(QSharedPointer<Thumbnail> newThumbnail)
{
    if (newThumbnail) {
        thumbnail = std::move(newThumbnail);
        isLoaded  = true;
        updateThumbnailDrawPosition();
        setupTextLayout();
        updateBackgroundRect();
        update();
    }
}

void ThumbnailWidgetCmp::unsetThumbnail()
{
    if (thumbnail)
        thumbnail.reset();
    isLoaded = false;
}

void ThumbnailWidgetCmp::setupTextLayout()
{
    if (thumbStyle == ThumbnailStyle::NORMAL || thumbStyle == ThumbnailStyle::NORMAL_CENTERED) {
        nameRect = QRect(padding + marginX, padding + marginY + mThumbnailSize + labelSpacing, mThumbnailSize, textHeight);
        infoRect = nameRect.adjusted(0, textHeight + 2, 0, textHeight + 2);
    } else if (thumbStyle == ThumbnailStyle::COMPACT) {
        nameRect = QRectF(marginX, marginY, width() - marginX * 2, textHeight * 1.7).toRect().adjusted(4, 0, -4, 0);
        if (thumbnail) {
            QFontMetrics fm(fontInfo);
            infoRect.setWidth(fm.horizontalAdvance(thumbnail->info()));
            infoRect.setHeight(nameRect.height());
            infoRect.moveTop(nameRect.top());
            infoRect.moveRight(nameRect.right());
            nameRect.adjust(0, 0, -infoRect.width() - 3, 0);
        }
    }
}

void ThumbnailWidgetCmp::updateBackgroundRect()
{
    bool verticalFit = (drawRectCentered.height() >= drawRectCentered.width());
    if (thumbStyle == ThumbnailStyle::NORMAL && !verticalFit) {
        bgRect.setBottom(height() - marginY);
        bgRect.setLeft(marginX);
        bgRect.setRight(width() - marginX);
        if (!thumbnail || !thumbnail->pixmap())
            bgRect.setTop(drawRectCentered.top() - padding);
        else // ensure we get equal padding on the top & sides
            bgRect.setTop(qMax(drawRectCentered.top() - drawRectCentered.left() + marginX, marginY));
    } else {
        bgRect = boundingRect().adjusted(marginX, marginY, -marginX, -marginY);
    }
}

void ThumbnailWidgetCmp::setHighlighted(bool mode)
{
    if (highlighted != mode) {
        highlighted = mode;
        update();
    }
}

bool ThumbnailWidgetCmp::isHighlighted() const
{
    return highlighted;
}

void ThumbnailWidgetCmp::setDropHovered(bool mode)
{
    if (dropHovered != mode) {
        dropHovered = mode;
        update();
    }
}

bool ThumbnailWidgetCmp::isDropHovered() const
{
    return dropHovered;
}

QRectF ThumbnailWidgetCmp::boundingRect() const
{
    return mBoundingRect;
}

void ThumbnailWidgetCmp::updateBoundingRect()
{
    mBoundingRect = QRectF(0, 0, mThumbnailSize + (padding + marginX) * 2, mThumbnailSize + (padding + marginY) * 2);
    if (thumbStyle == ThumbnailStyle::NORMAL || thumbStyle == ThumbnailStyle::NORMAL_CENTERED)
        mBoundingRect.adjust(0, 0, 0, labelSpacing + textHeight * 2);
}

qreal ThumbnailWidgetCmp::width() const
{
    return boundingRect().width();
}

qreal ThumbnailWidgetCmp::height() const
{
    return boundingRect().height();
}

void ThumbnailWidgetCmp::paint(QPainter *painter, QStyleOptionGraphicsItem const *, QWidget *)
{
    painter->setRenderHints(QPainter::Antialiasing);
    qreal dpr = painter->paintEngine()->paintDevice()->devicePixelRatioF();
    if (isHovered() && (!isHighlighted() || thumbStyle == ThumbnailStyle::COMPACT))
        drawHoverBg(painter);
    if (isHighlighted())
        drawHighlight(painter);

    if (!thumbnail) { // not loaded
        // todo: recolor once in shrRes
        QPixmap loadingIcon(*shrRes->getPixmap(ShrIcon::Loading, dpr));
        if (isHighlighted())
            ImageLib::recolor(loadingIcon, settings->colorScheme().accent);
        else
            ImageLib::recolor(loadingIcon, settings->colorScheme().folderview_hc2);
        drawIcon(painter, &loadingIcon);
    } else {
        if (thumbnail->pixmap()->width() == 0) { // invalid thumb
            QPixmap errorIcon(*shrRes->getPixmap(ShrIcon::Error, dpr));
            if (isHighlighted())
                ImageLib::recolor(errorIcon, settings->colorScheme().accent);
            else
                ImageLib::recolor(errorIcon, settings->colorScheme().folderview_hc2);
            drawIcon(painter, &errorIcon);
        } else {
            drawThumbnail(painter, thumbnail->pixmap().get());
            if (isHovered())
                drawHoverHighlight(painter);
        }
        if (thumbStyle != ThumbnailStyle::SIMPLE)
            drawLabel(painter);
    }
    if (isDropHovered())
        drawDropHover(painter);
}

void ThumbnailWidgetCmp::drawHighlight(QPainter *painter) const
{
    if (isHighlighted()) {
        if (thumbStyle == ThumbnailStyle::COMPACT) {
            painter->fillRect(QRectF(marginX, 0, width() - marginX * 2, marginY), settings->colorScheme().accent);
        } else {
            auto hints = painter->renderHints();
            auto op    = painter->opacity();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setOpacity(0.40f * op);
            painter->fillRect(bgRect, settings->colorScheme().accent);
            painter->setOpacity(0.70f * op);
            QPen pen(settings->colorScheme().accent, 2);
            painter->setPen(pen);
            painter->drawRect(bgRect.adjusted(1, 1, -1, -1)); // 2px pen
            // painter->drawRect(highlightRect.adjusted(0.5,0.5,-0.5,-0.5)); // 1px pen
            painter->setOpacity(op);
            painter->setRenderHints(hints);
        }
    }
}

void ThumbnailWidgetCmp::drawHoverBg(QPainter *painter) const
{
    auto op = painter->opacity();
    painter->fillRect(bgRect, settings->colorScheme().folderview_hc);
    painter->setOpacity(op);
}

void ThumbnailWidgetCmp::drawHoverHighlight(QPainter *painter) const
{
    auto op   = painter->opacity();
    auto mode = painter->compositionMode();
    painter->setCompositionMode(QPainter::CompositionMode_Plus);
    painter->setOpacity(0.2f);
    painter->drawPixmap(drawRectCentered, *thumbnail->pixmap());
    painter->setOpacity(op);
    painter->setCompositionMode(mode);
}

void ThumbnailWidgetCmp::drawLabel(QPainter *painter) const
{
    if (thumbnail) {
        bool centerName = true;
        if (thumbStyle == ThumbnailStyle::COMPACT) {
            painter->setOpacity(0.94);
            painter->fillRect(QRectF(marginX, nameRect.top(), width() - marginX * 2, nameRect.height()), QColor(20, 20, 20, 255));
            painter->setOpacity(1.0);
            centerName = false;
        }
        drawSingleLineText(painter, fontName, nameRect, thumbnail->name(), settings->colorScheme().text_hc2, centerName);
        auto op = painter->opacity();
        painter->setOpacity(op * 0.62f);
        drawSingleLineText(painter, fontInfo, infoRect, thumbnail->info(), settings->colorScheme().text_hc2);
        painter->setOpacity(op);
    }
}

void ThumbnailWidgetCmp::drawSingleLineText(
      QPainter      *painter,
      QFont const   &fnt,
      QRect          rect,
      QString const &text,
      QColor const  &color,
      bool           center)
{
    qreal dpr  = qApp->devicePixelRatio();
    auto  fm   = QFontMetrics(fnt);
    bool  fits = fm.horizontalAdvance(text) <= rect.width();

    // filename
    int flags;
    painter->setFont(fnt);
    if (fits) {
        if (center)
            flags = Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignHCenter;
        else
            flags = Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignLeft;
        painter->setPen(color);
        painter->drawText(rect, flags, text);
    } else {
        // fancy variant with text fade effect - uses temporary surface to paint; slow
        QPixmap textLayer(rect.width() * dpr, rect.height() * dpr);
        textLayer.fill(Qt::transparent);
        textLayer.setDevicePixelRatio(dpr);
        QPainter textPainter(&textLayer);
        textPainter.setFont(fnt);
        // paint text onto tmp layer
        flags = Qt::TextSingleLine | Qt::AlignVCenter;
        textPainter.setPen(color);
        QRect textRect = QRect(0, 0, rect.width(), rect.height());
        textPainter.drawText(textRect, flags, text);
        QRectF fadeRect = textRect.adjusted(textRect.width() - 6, 0, 0, 0);
        // fade effect
        QLinearGradient gradient(fadeRect.topLeft(), fadeRect.topRight());
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::red); // any color, this is just a transparency mask
        textPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        textPainter.fillRect(fadeRect, gradient);
        // write text layer into graphicsitem
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->drawPixmap(rect.topLeft(), textLayer);
    }
}

void ThumbnailWidgetCmp::drawDropHover(QPainter *painter) const
{
    // save
    auto hints = painter->renderHints();
    auto op    = painter->opacity();

    painter->setRenderHint(QPainter::Antialiasing);
    QColor clr(190, 60, 25);
    painter->setOpacity(0.1f * op);
    painter->fillRect(bgRect, clr);
    painter->setOpacity(op);
    QPen pen(clr, 2);
    painter->setPen(pen);
    painter->drawRect(bgRect.adjusted(1, 1, -1, -1));
    painter->setRenderHints(hints);
}

void ThumbnailWidgetCmp::drawThumbnail(QPainter *painter, QPixmap const *pixmap) const
{
    if (!thumbnail->hasAlphaChannel() && thumbStyle != ThumbnailStyle::COMPACT)
        painter->fillRect(drawRectCentered.adjusted(3, 3, 3, 3), shadowColor);
    painter->drawPixmap(drawRectCentered, *pixmap);
}

void ThumbnailWidgetCmp::drawIcon(QPainter *painter, QPixmap const *pixmap) const
{
    QPointF drawPosCentered(width() / 2 - pixmap->width() / (2 * pixmap->devicePixelRatioF()),
                            height() / 2 - pixmap->height() / (2 * pixmap->devicePixelRatioF()));
    painter->drawPixmap(drawPosCentered, *pixmap, QRect({0, 0}, pixmap->size()));
}

QSizeF ThumbnailWidgetCmp::sizeHint(Qt::SizeHint, QSizeF const &) const
{
    return boundingRect().size();
}

void ThumbnailWidgetCmp::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    setHovered(true);
}

void ThumbnailWidgetCmp::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    setHovered(false);
}

void ThumbnailWidgetCmp::setHovered(bool mode)
{
    if (hovered != mode) {
        hovered = mode;
        update();
    }
}

bool ThumbnailWidgetCmp::isHovered() const
{
    return hovered;
}

void ThumbnailWidgetCmp::updateThumbnailDrawPosition()
{
    if (thumbnail) {
        // dpr-adjusted size
        QSize pixmapSize = isLoaded ? thumbnail->pixmap()->size() / qApp->devicePixelRatio()
                                    : thumbnail->pixmap()->size().scaled(mThumbnailSize, mThumbnailSize, Qt::KeepAspectRatio);
        QPoint topLeft;
        bool   verticalFit = (pixmapSize.height() >= pixmapSize.width());
        if (thumbStyle == ThumbnailStyle::SIMPLE || thumbStyle == ThumbnailStyle::COMPACT) {
            topLeft.setX((width() - pixmapSize.width()) / 2.0);
            topLeft.setY((height() - pixmapSize.height()) / 2.0);
        } else if (thumbStyle == ThumbnailStyle::NORMAL_CENTERED && !verticalFit) {
            topLeft.setX((width() - pixmapSize.width()) / 2.0);
            topLeft.setY((height() - pixmapSize.height()) / 2.0 - textHeight);
        } else { // ThumbnailStyle::NORMAL
            // snap thumbnail to the filename label
            topLeft.setX((width() - pixmapSize.width()) / 2.0);
            topLeft.setY(padding + marginY + mThumbnailSize - pixmapSize.height());
        }
        drawRectCentered = QRect(topLeft, pixmapSize);
    }
}

ThumbnailWidgetCmp::~ThumbnailWidgetCmp() = default;
