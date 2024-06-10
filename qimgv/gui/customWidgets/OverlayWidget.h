/* Base class for floating widgets.
 * It will automatically reposition itself according to FloatingWidgetPosition.
 */

#pragma once

#include "gui/customWidgets/FloatingWidget.h"
#include <QTimeLine>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDebug>

enum class FloatingWidgetPosition {
    LEFT,
    RIGHT,
    BOTTOM,
    TOP,
    TOPLEFT,
    TOPRIGHT,
    BOTTOMLEFT,
    BOTTOMRIGHT,
    CENTER,
};

class OverlayWidget : public FloatingWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

  public:
    explicit OverlayWidget(FloatingWidgetContainer *parent);
    ~OverlayWidget() override;

    ND int horizontalMargin() const;
    ND int verticalMargin() const;

    void setHorizontalMargin(int);
    void setVerticalMargin(int);
    void setPosition(FloatingWidgetPosition pos);
    void setFadeDuration(int duration);
    void setFadeEnabled(bool mode);

  public slots:
    void show();
    void hide();
    void hideAnimated();

  private:
    QGraphicsOpacityEffect *opacityEffect;
    QPropertyAnimation     *fadeAnimation;

    int  mHorizontalMargin;
    int  mVerticalMargin;
    bool fadeEnabled;

  private slots:
    void setOpacity(qreal opacity);
    ND qreal opacity() const;

  protected:
    void recalculateGeometry() override;

    FloatingWidgetPosition position;
};
