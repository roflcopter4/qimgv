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

enum class ThumbnailStyle : uint8_t {
    SIMPLE,
    NORMAL,
    NORMAL_CENTERED,
};

class ThumbnailWidget : public QGraphicsWidget
{
    Q_OBJECT

  public:
    explicit ThumbnailWidget(QGraphicsItem *parent = nullptr);
    ~ThumbnailWidget() override;

    enum { Type = UserType + 1 };
    ND int type() const override { return Type; }

       void setThumbnail(QSharedPointer<Thumbnail> const &newThumbnail);
       void setHighlighted(bool mode);
    ND bool isHighlighted() const;
       void setDropHovered(bool mode);
    ND bool isDropHovered() const;
       void setThumbnailSize(int size);
       void setGeometry(QRectF const &rect) override;
       void setThumbStyle(ThumbnailStyle style);
       void setPadding(int newPadding);
       void setMargins(int newMarginX, int newMarginY);
    ND int  thumbnailSize() const;
       void reset();
       void unsetThumbnail();

    ND qreal  width() const;
    ND qreal  height() const;
    ND QRectF boundingRect() const override;
    ND QSizeF effectiveSizeHint(Qt::SizeHint which, QSizeF const &constraint = QSizeF()) const;

    ND virtual QRectF geometry() const;

  protected:
    void setupTextLayout();
    void drawThumbnail(QPainter *painter, QPixmap const *pixmap);
    void drawIcon(QPainter *painter, QPixmap const *pixmap);
    void drawHighlight(QPainter *painter);
    void drawHoverBg(QPainter *painter);
    void drawHoverHighlight(QPainter *painter);
    void drawLabel(QPainter *painter);
    void drawDropHover(QPainter *painter);
    void drawSingleLineText(QPainter *painter, QRect rect, QString const &text, QColor const &color);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *item, QWidget *widget) override;
    void updateGeometry() override;
    void updateBackgroundRect();
    void updateThumbnailDrawPosition();
    void updateBoundingRect();
    void setHovered(bool);

    ND bool   isHovered() const;
    ND QSizeF sizeHint(Qt::SizeHint which, QSizeF const &constraint = QSizeF()) const override;

    QSharedPointer<Thumbnail> thumbnail;

    QRectF bgRect;
    QRectF mBoundingRect;
    QFont  font;
    QFont  fontInfo;
    QRect  drawRectCentered;
    QRect  nameRect;
    QRect  infoRect;
    int    mThumbnailSize;
    int    padding;
    int    marginX;
    int    marginY;
    int    labelSpacing;
    int    textHeight;
    bool   highlighted;
    bool   hovered;
    bool   dropHovered;

    ThumbnailStyle thumbStyle;

  public:
    bool isLoaded;
};
