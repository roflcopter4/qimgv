#include "mapoverlay.h"
#include "settings.h"
#include <QPropertyAnimation>

#include "Common.h"

class MapOverlay::MapOverlayPrivate final : public QObject
{
  public:
    explicit MapOverlayPrivate(MapOverlay *qq);
    ~MapOverlayPrivate() override;

    void moveInnerWidget(qreal x, qreal y);
    void moveMainImage(qreal xPos, qreal yPos);

    friend class MapOverlay;

    DELETE_COPY_MOVE_CONSTRUCTORS(MapOverlayPrivate);

  private:
    MapOverlay *q;

    QPen     outlinePen;
    QRectF   outerRect;
    QRectF   innerRect;
    QRectF   windowRect;
    QRectF   drawingRect;
    int      size        = 120;
    int      margin      = 20;
    qreal    xSpeedDiff  = 0.0;
    qreal    ySpeedDiff  = 0.0;
    qreal    opacity     = 0.0;
    qreal    innerOffset = -1.0;
    Location location    = Location::RightBottom;

    QPropertyAnimation *opacityAnimation    = nullptr;
    QPropertyAnimation *transitionAnimation = nullptr;
};

MapOverlay::MapOverlayPrivate::MapOverlayPrivate(MapOverlay *qq)
    : q(qq)
{
    outlinePen.setColor(QColor(180, 180, 180, 255));
}

MapOverlay::MapOverlayPrivate::~MapOverlayPrivate()
{
    delete opacityAnimation;
    delete transitionAnimation;
}

void MapOverlay::MapOverlayPrivate::moveInnerWidget(qreal x, qreal y)
{
    if (x + innerRect.width() > outerRect.right())
        x = outerRect.right() - innerRect.width();

    if (x < 0)
        x = 0;

    if (y + innerRect.height() > outerRect.bottom())
        y = outerRect.bottom() - innerRect.height();

    if (y < 0)
        y = 0;

    innerRect.moveTo(QPointF(x, y));
    q->update();
}

void MapOverlay::MapOverlayPrivate::moveMainImage(qreal xPos, qreal yPos)
{
    qreal x = xPos - innerRect.width()  / 2.0;
    qreal y = yPos - innerRect.height() / 2.0;

    moveInnerWidget(x, y);

    x /= -xSpeedDiff;
    y /= -ySpeedDiff;

    // Check limits;
    qreal invisibleX = windowRect.width() - drawingRect.width();
    qreal invisibleY = windowRect.height() - drawingRect.height();

    if (x < invisibleX)
        x = invisibleX;
    if (x > 0)
        x = 0;

    if (y < invisibleY)
        y = invisibleY;
    if (y > 0)
        y = 0;

    emit q->positionChanged(x, y);
}

/****************************************************************************************/

MapOverlay::MapOverlay(QWidget *parent)
    : QWidget(parent),
      d(new MapOverlayPrivate(this)),
      visibilityEnabled(true)
{
    setMouseTracking(true);

    d->opacityAnimation = new QPropertyAnimation(this, "opacity");
    d->opacityAnimation->setEasingCurve(QEasingCurve::OutSine);
    d->opacityAnimation->setDuration(150);

    d->transitionAnimation = new QPropertyAnimation(this, "y");
    d->transitionAnimation->setDuration(200);
    d->transitionAnimation->setEasingCurve(QEasingCurve::OutExpo);

    QWidget::setVisible(true);
}

MapOverlay::~MapOverlay()
{
    delete d;
}

QSizeF MapOverlay::inner() const
{
    return d->innerRect.size();
}

QSizeF MapOverlay::outer() const
{
    return d->outerRect.size();
}

qreal MapOverlay::opacity() const
{
    return d->opacity;
}

void MapOverlay::enableVisibility(bool mode)
{
    visibilityEnabled = mode;
}

void MapOverlay::setOpacity(qreal opacity)
{
    d->opacity = opacity;
    update();
}

void MapOverlay::animateVisible(bool isVisible)
{
    if (isVisible)
        this->setOpacity(1.0f);
    else {
        d->opacityAnimation->setEndValue(1.0f * isVisible);

        switch (location()) {
        case Location::LeftTop:
        case Location::RightTop:
            if (isVisible) {
                d->transitionAnimation->setStartValue(0);
                d->transitionAnimation->setEndValue(margin());
            } else {
                d->transitionAnimation->setStartValue(margin());
                d->transitionAnimation->setEndValue(0);
            }
            break;
        case Location::RightBottom:
        case Location::LeftBottom:
            int h      = parentWidget()->height();
            int offset = d->outerRect.height() + margin();

            if (isVisible) {
                d->transitionAnimation->setStartValue(h);
                d->transitionAnimation->setEndValue(h - offset);
            } else {
                d->transitionAnimation->setStartValue(h - offset);
                d->transitionAnimation->setEndValue(h);
            }
            break;
        }

        d->opacityAnimation->start();
        // d->transitionAnimation->start();
    }
    // if (QWidget::isVisible() == isVisible) // already in this state
    //     return;
}

void MapOverlay::resize(int size)
{
    QWidget::resize(size, size);
}

void MapOverlay::setY(int y)
{
    move(x(), y);
}

int MapOverlay::y() const
{
    return QWidget::y();
}

void MapOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    QBrush   outerBrush(QColor(40, 40, 40, 160), Qt::SolidPattern);
    QBrush   innerBrush(QColor(180, 180, 180, 255), Qt::SolidPattern);

    painter.setOpacity(d->opacity);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.fillRect(d->outerRect, outerBrush);
    painter.fillRect(d->innerRect, innerBrush);

    painter.setPen(d->outlinePen);
    painter.drawRect(d->outerRect);
}

void MapOverlay::updatePosition()
{
    QRect parentRect = parentWidget()->rect();

    int x = 0, y = 0;
    switch (location()) {
    case Location::LeftTop:
        x = parentRect.left() + margin();
        y = parentRect.top() + margin();
        break;
    case Location::RightTop:
        x = parentRect.right() - (margin() + d->outerRect.width());
        y = parentRect.top() + margin();
        break;
    case Location::RightBottom:
        x = parentRect.right() - (margin() + d->outerRect.width());
        y = parentRect.bottom() - (margin() + d->outerRect.height());
        break;
    case Location::LeftBottom:
        x = parentRect.left() + margin();
        y = parentRect.bottom() - (margin() + d->outerRect.height());
        break;
    }

    /**
     * Save extra space for outer rect border
     * one pixel for right and bottom sides
     */
    setGeometry(x, y, d->size + 1, d->size + 1);
}

bool contains(const QRectF &real, const QRectF &expected)
{
    return real.width() <= expected.width() && real.height() <= expected.height();
}

void MapOverlay::updateMap(const QRectF &drawingRect)
{
    if (!isEnabled())
        return;

    QRectF windowRect = parentWidget()->rect();

    imageDoesNotFit = !contains(drawingRect, windowRect);
    animateVisible(imageDoesNotFit && visibilityEnabled);

    /**
     * Always calculate this first for properly map location
     */
    QSizeF outerSz = drawingRect.size();
    outerSz.scale(d->size, d->size, Qt::KeepAspectRatio);
    d->outerRect.setSize(outerSz);

    d->windowRect  = windowRect;
    d->drawingRect = drawingRect;

    qreal aspect = outerSz.width() / drawingRect.width();
    qreal innerWidth = std::min(windowRect.width() * aspect, outerSz.width());
    qreal innerHeight = std::min(windowRect.height() * aspect, outerSz.height());

    QSizeF innerSz(innerWidth, innerHeight);
    d->innerRect.setSize(innerSz);

    d->xSpeedDiff = innerSz.width() / windowRect.width();
    d->ySpeedDiff = innerSz.height() / windowRect.height();

    qreal x = -drawingRect.left() * d->xSpeedDiff;
    qreal y = -drawingRect.top() * d->ySpeedDiff;

    d->moveInnerWidget(x, y);
    update();
}

void MapOverlay::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    setCursor(Qt::ClosedHandCursor);
    auto pos = event->position();
    d->moveMainImage(pos.x(), pos.y());
    event->accept();
}

void MapOverlay::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (event->buttons() & Qt::LeftButton) {
        auto pos = event->position();
        d->moveMainImage(pos.x(), pos.y());
    }
    event->accept();
}

void MapOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    setCursor(Qt::ArrowCursor);
    event->accept();
}

void MapOverlay::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updatePosition();
}

void MapOverlay::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    this->enableVisibility(false);
    this->animateVisible(false);
    this->update();
}

void MapOverlay::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    this->enableVisibility(isVisible());
    this->animateVisible(visibilityEnabled && imageDoesNotFit);
    this->update();
}

int MapOverlay::size() const
{
    return d->size;
}

MapOverlay::Location MapOverlay::location() const
{
    return d->location;
}

void MapOverlay::setLocation(MapOverlay::Location loc)
{
    d->location = loc;
}

int MapOverlay::margin() const
{
    return d->margin;
}

void MapOverlay::setMargin(int margin)
{
    d->margin = margin;
}
