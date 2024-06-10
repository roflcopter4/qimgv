#include "ThumbnailWidget.h"

ThumbnailWidget::ThumbnailWidget(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      thumbnail(nullptr),
      mThumbnailSize(100),
      padding(5),
      marginX(2),
      marginY(2),
      labelSpacing(9),
      textHeight(5),
      highlighted(false),
      hovered(false),
      dropHovered(false),
      thumbStyle(ThumbnailStyle::SIMPLE),
      isLoaded(false)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    qreal dpr = qApp->devicePixelRatio();
    if (trunc(dpr) == dpr) // don't enable for fractional scaling
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    font.setBold(false);
    QFontMetrics fm(font);
    textHeight = fm.height();
}

ThumbnailWidget::~ThumbnailWidget()
{
}

void ThumbnailWidget::setThumbnailSize(int size)
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

void ThumbnailWidget::setPadding(int newPadding)
{
    padding = newPadding;
    updateBoundingRect();
}

void ThumbnailWidget::setMargins(int newMarginX, int newMarginY)
{
    marginX = newMarginX;
    marginY = newMarginY;
    updateBoundingRect();
}

int ThumbnailWidget::thumbnailSize() const
{
    return mThumbnailSize;
}

void ThumbnailWidget::reset()
{
    if (thumbnail)
        thumbnail.reset();
    highlighted = false;
    hovered     = false;
    isLoaded    = false;
    update();
}

void ThumbnailWidget::setThumbStyle(ThumbnailStyle style)
{
    if (thumbStyle != style) {
        thumbStyle = style;
        updateBoundingRect();
        updateThumbnailDrawPosition();
        setupTextLayout();
        updateBackgroundRect();
        updateGeometry();
        update();
    }
}

void ThumbnailWidget::updateGeometry()
{
    QGraphicsWidget::updateGeometry();
}

void ThumbnailWidget::setGeometry(QRectF const &rect)
{
    QGraphicsWidget::setGeometry(QRectF(rect.topLeft(), boundingRect().size()));
}

QRectF ThumbnailWidget::geometry() const
{
    return {QGraphicsWidget::geometry().topLeft(), boundingRect().size()};
}

QSizeF ThumbnailWidget::effectiveSizeHint(Qt::SizeHint which, QSizeF const &constraint) const
{
    return sizeHint(which, constraint);
}

void ThumbnailWidget::setThumbnail(QSharedPointer<Thumbnail> const &newThumbnail)
{
    if (newThumbnail) {
        thumbnail = newThumbnail;
        isLoaded  = true;
        updateThumbnailDrawPosition();
        setupTextLayout();
        updateBackgroundRect();
        update();
    }
}

void ThumbnailWidget::unsetThumbnail()
{
    if (thumbnail)
        thumbnail.reset();
    isLoaded = false;
}

void ThumbnailWidget::setupTextLayout()
{
    if (thumbStyle != ThumbnailStyle::SIMPLE) {
        nameRect = QRect(padding + marginX, padding + marginY + mThumbnailSize + labelSpacing, mThumbnailSize, textHeight);
        infoRect = nameRect.adjusted(0, textHeight + 2, 0, textHeight + 2);
    }
}

void ThumbnailWidget::updateBackgroundRect()
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

void ThumbnailWidget::setHighlighted(bool mode)
{
    if (highlighted != mode) {
        highlighted = mode;
        update();
    }
}

bool ThumbnailWidget::isHighlighted() const
{
    return highlighted;
}

void ThumbnailWidget::setDropHovered(bool mode)
{
    if (dropHovered != mode) {
        dropHovered = mode;
        update();
    }
}

bool ThumbnailWidget::isDropHovered() const
{
    return dropHovered;
}

QRectF ThumbnailWidget::boundingRect() const
{
    return mBoundingRect;
}

void ThumbnailWidget::updateBoundingRect()
{
    mBoundingRect = QRectF(0, 0, mThumbnailSize + (padding + marginX) * 2, mThumbnailSize + (padding + marginY) * 2);
    if (thumbStyle != ThumbnailStyle::SIMPLE)
        mBoundingRect.adjust(0, 0, 0, labelSpacing + textHeight * 2);
}

qreal ThumbnailWidget::width() const
{
    return boundingRect().width();
}

qreal ThumbnailWidget::height() const
{
    return boundingRect().height();
}

void ThumbnailWidget::paint(QPainter *painter, QStyleOptionGraphicsItem const *, QWidget *)
{
    painter->setRenderHints(QPainter::Antialiasing);
    qreal dpr = painter->paintEngine()->paintDevice()->devicePixelRatioF();
    if (isHovered() && !isHighlighted())
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
        if (!thumbnail->pixmap() || thumbnail->pixmap()->width() == 0) { // invalid thumb
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

void ThumbnailWidget::drawHighlight(QPainter *painter)
{
    if (isHighlighted()) {
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

void ThumbnailWidget::drawHoverBg(QPainter *painter)
{
    auto op = painter->opacity();
    painter->fillRect(bgRect, settings->colorScheme().folderview_hc);
    painter->setOpacity(op);
}

void ThumbnailWidget::drawHoverHighlight(QPainter *painter)
{
    auto op   = painter->opacity();
    auto mode = painter->compositionMode();
    painter->setCompositionMode(QPainter::CompositionMode_Plus);
    painter->setOpacity(0.2f);
    painter->drawPixmap(drawRectCentered, *thumbnail->pixmap());
    painter->setOpacity(op);
    painter->setCompositionMode(mode);
}

void ThumbnailWidget::drawLabel(QPainter *painter)
{
    if (thumbnail) {
        drawSingleLineText(painter, nameRect, thumbnail->name(), settings->colorScheme().text_hc2);
        auto op = painter->opacity();
        painter->setOpacity(op * 0.62f);
        drawSingleLineText(painter, infoRect, thumbnail->info(), settings->colorScheme().text_hc2);
        painter->setOpacity(op);
    }
}

void ThumbnailWidget::drawSingleLineText(QPainter *painter, QRect rect, QString const &text, QColor const &color)
{
    qreal        dpr = qApp->devicePixelRatio();
    QFontMetrics fm(font);
    bool         fits = !(fm.horizontalAdvance(text) > rect.width());
    // filename
    int flags;
    painter->setFont(font);
    if (fits) {
        flags = Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignHCenter;
        painter->setPen(color);
        painter->drawText(rect, flags, text);
    } else {
        // fancy variant with text fade effect - uses temporary surface to paint; slow
        QPixmap textLayer(rect.width() * dpr, rect.height() * dpr);
        textLayer.fill(Qt::transparent);
        textLayer.setDevicePixelRatio(dpr);
        QPainter textPainter(&textLayer);
        textPainter.setFont(font);
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

void ThumbnailWidget::drawDropHover(QPainter *painter)
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

void ThumbnailWidget::drawThumbnail(QPainter *painter, QPixmap const *pixmap)
{
    if (!thumbnail->hasAlphaChannel())
        painter->fillRect(drawRectCentered.adjusted(3, 3, 3, 3), QColor(0, 0, 0, 60));
    painter->drawPixmap(drawRectCentered, *pixmap);
}

void ThumbnailWidget::drawIcon(QPainter *painter, QPixmap const *pixmap)
{
    QPointF drawPosCentered(width()  / 2.0 - pixmap->width()  / (2.0 * pixmap->devicePixelRatioF()),
                            height() / 2.0 - pixmap->height() / (2.0 * pixmap->devicePixelRatioF()));
    painter->drawPixmap(drawPosCentered, *pixmap, QRectF(QPoint(0, 0), pixmap->size()));
}

QSizeF ThumbnailWidget::sizeHint(Qt::SizeHint, QSizeF const &) const
{
    return boundingRect().size();
}

void ThumbnailWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    setHovered(true);
}

void ThumbnailWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    setHovered(false);
}

void ThumbnailWidget::setHovered(bool mode)
{
    if (hovered != mode) {
        hovered = mode;
        update();
    }
}

bool ThumbnailWidget::isHovered() const
{
    return hovered;
}

void ThumbnailWidget::updateThumbnailDrawPosition()
{
    if (thumbnail && thumbnail->pixmap()) {
        QPoint topLeft;
        QSize  pixmapSize; // dpr-adjusted size
        if (isLoaded)
            pixmapSize = thumbnail->pixmap()->size() / qApp->devicePixelRatio();
        else
            pixmapSize = thumbnail->pixmap()->size().scaled(mThumbnailSize, mThumbnailSize, Qt::KeepAspectRatio);

        bool verticalFit = (pixmapSize.height() >= pixmapSize.width());
        topLeft.setX((width() - pixmapSize.width()) / 2.0);

        if (thumbStyle == ThumbnailStyle::SIMPLE)
            topLeft.setY((height() - pixmapSize.height()) / 2.0);
        else if (thumbStyle == ThumbnailStyle::NORMAL_CENTERED && !verticalFit)
            topLeft.setY((height() - pixmapSize.height()) / 2.0 - textHeight);
        else // NORMAL - snap thumbnail to the filename label
            topLeft.setY(padding + marginY + mThumbnailSize - pixmapSize.height());

        drawRectCentered = QRect(topLeft, pixmapSize);
    }
}
