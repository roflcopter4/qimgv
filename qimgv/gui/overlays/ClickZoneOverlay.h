#pragma once

#include "gui/customwidgets/floatingwidget.h"
#include "utils/imagelib.h"
#include <QDebug>
#include <QGraphicsOpacityEffect>

class ClickZoneOverlay : public FloatingWidget
{
    Q_OBJECT

    enum class ActiveHighlightZone : uint8_t {
        NONE,
        LEFT,
        RIGHT,
    };

  public:
    explicit ClickZoneOverlay(FloatingWidgetContainer *parent);
    ~ClickZoneOverlay() override;
    DELETE_COPY_MOVE_ROUTINES(ClickZoneOverlay);

    ND QRectF leftZone() const;
    ND QRectF rightZone() const;
    ND bool   isHighlighted() const;

    void  highlightLeft();
    void  highlightRight();
    void  disableHighlight();
    void  setHighlightedZone(ActiveHighlightZone zone);
    void  setPressed(bool mode);

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void recalculateGeometry() override;

  private:
    ND QPixmap *loadPixmap(QString path);
    void drawPixmap(QPainter &p, QPixmap const *pixmap, QRectF const &rect) const;

  public Q_SLOTS:
    void readSettings();

  private:
    static constexpr int zoneSize = 110;

    QPixmap *pixmapLeft  = nullptr;
    QPixmap *pixmapRight = nullptr;
    QRectF   mLeftZone;
    QRectF   mRightZone;
    qreal    dpr;
    qreal    pixmapDrawScale;

    bool hiResPixmaps  = false;
    bool isPressed     = false;
    bool leftHovered   = false;
    bool rightHovered  = false;
    bool drawZones     = true;

    ActiveHighlightZone activeZone = ActiveHighlightZone::NONE;
};
