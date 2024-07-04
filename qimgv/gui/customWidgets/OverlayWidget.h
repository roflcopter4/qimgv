/* Base class for floating widgets.
 * It will automatically reposition itself according to FloatingWidgetPosition.
 */

#pragma once

#include "gui/customWidgets/FloatingWidget.h"
#include <QTimeLine>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDebug>

class OverlayWidget : public FloatingWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

  public:
    explicit OverlayWidget(FloatingWidgetContainer *parent);
    ~OverlayWidget() override;
    DELETE_COPY_MOVE_ROUTINES(OverlayWidget);

    void setHorizontalMargin(int);
    void setVerticalMargin(int);
    void setPosition(FloatingWidget::Position pos);
    void setFadeDuration(int duration);
    void setFadeEnabled(bool mode);

    ND int horizontalMargin() const;
    ND int verticalMargin() const;

  public Q_SLOTS:
    void show();
    void hide();
    void hideAnimated();

  private Q_SLOTS:
    void setOpacity(qreal opacity);
    ND qreal opacity() const;

  protected:
    void recalculateGeometry() override;

  private:
    QGraphicsOpacityEffect *opacityEffect;
    QPropertyAnimation     *fadeAnimation;

    int  mHorizontalMargin = 20;
    int  mVerticalMargin   = 35;
    bool fadeEnabled       = false;

    FloatingWidget::Position position = FloatingWidget::Position::Bottom;
};
