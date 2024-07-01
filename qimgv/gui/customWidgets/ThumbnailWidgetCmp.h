#pragma once

#include "Settings.h"
#include "SharedResources.h"
#include "sourcecontainers/Thumbnail.h"
#include "utils/ImageLib.h"
#include <QGraphicsItem>
#include <QGraphicsLayoutItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsWidget>
#include <QMouseEvent>
#include <QPaintEngine>
#include <QPainter>
#include <cmath>

enum class ThumbnailStyle {
    SIMPLE,
    NORMAL,
    NORMAL_CENTERED,
    COMPACT,
};

class ThumbnailWidgetCmp final : public QGraphicsWidget
{
    Q_OBJECT

  public:
    explicit ThumbnailWidgetCmp(QGraphicsItem *parent = nullptr);
    ~ThumbnailWidgetCmp() override;
    DELETE_COPY_MOVE_ROUTINES(ThumbnailWidgetCmp);

    enum { Type = UserType + 1 };
    ND int type() const override { return Type; }

    ND QRectF geometry() const;
       void   setGeometry(QRectF const &rect) override;
    ND int    thumbnailSize() const;
       void   setThumbnailSize(int size);
    ND bool   isDropHovered() const;
       void   setDropHovered(bool mode);
    ND bool   isHighlighted() const;
       void   setHighlighted(bool mode);

    ND QRectF boundingRect() const override;
    ND QSizeF effectiveSizeHint(Qt::SizeHint which, QSizeF const &constraint = QSizeF()) const;
    ND qreal  height() const;
    ND qreal  width() const;

    void setThumbnail(QSharedPointer<Thumbnail> newThumbnail);
    void setThumbStyle(ThumbnailStyle style);
    void setPadding(int newPadding);
    void setMargins(int newMarginX, int newMarginY);
    void unsetThumbnail();
    void reset();

  protected:
    void setupTextLayout();
    void drawThumbnail(QPainter *painter, QPixmap const *pixmap) const;
    void drawIcon(QPainter *painter, QPixmap const *pixmap) const;
    void drawHighlight(QPainter *painter) const;
    void drawHoverBg(QPainter *painter) const;
    void drawHoverHighlight(QPainter *painter) const;
    void drawLabel(QPainter *painter) const;
    void drawDropHover(QPainter *painter) const;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget) override;
    void updateGeometry() override;
    void setHovered(bool);
    void updateBackgroundRect();
    void updateThumbnailDrawPosition();
    void updateBoundingRect();

    ND bool   isHovered() const;
    ND QSizeF sizeHint(Qt::SizeHint which, QSizeF const &constraint = QSizeF()) const override;

    static void drawSingleLineText(QPainter *painter, QFont const &fnt, QRect rect, QString const &text, QColor const &color, bool center = true);

   private:
    QSharedPointer<Thumbnail> thumbnail = nullptr;

    QRectF bgRect;
    QRectF mBoundingRect;
    QColor shadowColor;
    QFont  fontName;
    QFont  fontInfo;
    QRect  drawRectCentered;
    QRect  nameRect;
    QRect  infoRect;
    int    mThumbnailSize = 100;
    int    padding        = 5;
    int    marginX        = 2;
    int    marginY        = 2;
    int    labelSpacing   = 9;
    int    textHeight     = 5;
    bool   highlighted    = false;
    bool   hovered        = false;
    bool   dropHovered    = false;

    ThumbnailStyle thumbStyle = ThumbnailStyle::SIMPLE;

  public:
    bool isLoaded = false;
};
