#pragma once

#include "floatingwidget.h"
#include "settings.h"
#include <QBoxLayout>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QTimeLine>
#include <QTimer>
#include <QtGlobal>
#include <ctime>
#include <memory>

#include "Common.h"


class SlidePanel : public FloatingWidget
{
    Q_OBJECT

  public:
    explicit SlidePanel(FloatingWidgetContainer *parent);
    ~SlidePanel() override;

    ND bool hasWidget() const;
    void setWidget(QSharedPointer<QWidget> const &w);
    // Use visibleGeometry instead of geometry() here.
    // If this is called mid-animation then geometry() will be all wrong.
    ND QRect triggerRect() const;
    // when this is set, the widget will not change geometry by itself
    // no pos() animations & no recalculateGeometry()
    ND bool layoutManaged() const;
    void    setLayoutManaged(bool mode);

    virtual void setPosition(PanelPosition);
    ND PanelPosition position() const;
    void hideAnimated();

  public slots:
    void show();
    void hide();

  private slots:
    void onAnimationFinish();
    void animationUpdate(int frame);

  protected:
    static constexpr unsigned ANIMATION_DURATION = 230;

    void     setAnimationRange(QPoint start, QPoint end);
    void     saveStaticGeometry(QRect geometry);
    void     recalculateGeometry() override;
    ND QRect staticGeometry() const;

    virtual void updateTriggerRect();

    int           panelSize;
    int           slideAmount;
    PanelPosition mPosition;
    QHBoxLayout   *mLayout;
    QRect         mTriggerRect;
    QTimer        timer;
    QTimeLine     timeline;
    QEasingCurve  outCurve;

    QGraphicsOpacityEffect  *fadeEffect;
    //QSharedPointer<QWidget> mWidget;
    QWidget *mWidget;

  private:
    void setOrientation();

    QRect           mStaticGeometry;
    QPoint          startPosition;
    QPoint          endPosition;
    qreal           panelVisibleOpacity = 1.0;
    Qt::Orientation mOrientation;
    bool            mLayoutManaged = false;
};
