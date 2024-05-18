#pragma once

#include <QGraphicsWidget>
#include <QGraphicsItem>
#include <QGraphicsLayoutItem>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QPaintEngine>
#include <cmath>
#include "sourcecontainers/thumbnail.h"
#include "utils/imagelib.h"
#include "settings.h"
#include "sharedresources.h"

#include "Common.h"

enum class ThumbnailStyle {
    SIMPLE,
    NORMAL,
    NORMAL_CENTERED,
    COMPACT,
};

class ThumbnailWidgetCmp : public QGraphicsWidget
{
    Q_OBJECT

  public:
    explicit ThumbnailWidgetCmp(QGraphicsItem *parent = nullptr);
    ~ThumbnailWidgetCmp() override;

    enum { Type = UserType + 1 };
    ND int type() const override { return Type; }

    void  setThumbnail(std::shared_ptr<Thumbnail> _thumbnail);
    void  setHighlighted(bool mode);
    bool  isHighlighted();
    void  setDropHovered(bool mode);
    bool  isDropHovered();
    qreal width();
    qreal height();
    void  setThumbnailSize(int size);
    void  setGeometry(QRectF const &rect) override;

    ND virtual QRectF geometry() const;

    ND QRectF boundingRect() const override;
    ND QSizeF effectiveSizeHint(Qt::SizeHint which, QSizeF const &constraint = QSizeF()) const;

    void setThumbStyle(ThumbnailStyle _style);
    void setPadding(int _padding);
    void setMargins(int _marginX, int _marginY);
    int  thumbnailSize();
    void reset();
    void unsetThumbnail();

  protected:
    void setupTextLayout();
    void drawThumbnail(QPainter *painter, QPixmap const *pixmap);
    void drawIcon(QPainter *painter, QPixmap const *pixmap);
    void drawHighlight(QPainter *painter);
    void drawHoverBg(QPainter *painter);
    void drawHoverHighlight(QPainter *painter);
    void drawLabel(QPainter *painter);
    void drawDropHover(QPainter *painter);
    void drawSingleLineText(QPainter *painter, QFont &_fnt, QRect rect, QString text, QColor const &color, bool center = true);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *item, QWidget *widget) override;
    void updateGeometry() override;
    void setHovered(bool);
    bool isHovered();
    void updateBackgroundRect();
    void updateThumbnailDrawPosition();
    void updateBoundingRect();

    ND QSizeF sizeHint(Qt::SizeHint which, QSizeF const &constraint = QSizeF()) const override;

    std::shared_ptr<Thumbnail> thumbnail;

    QRectF bgRect;
    QRectF mBoundingRect;
    QColor shadowColor;
    QFont  fontName;
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
