#pragma once

#include "gui/customWidgets/ActionButton.h"
#include "gui/customWidgets/FloatingWidget.h"
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPropertyAnimation>

class ControlsOverlay : public FloatingWidget
{
    Q_OBJECT

  public:
    explicit ControlsOverlay(FloatingWidgetContainer *parent);

  public slots:
    void show();

  private:
    QHBoxLayout            *layout;
    ActionButton           *closeButton;
    ActionButton           *settingsButton;
    ActionButton           *folderViewButton;
    QGraphicsOpacityEffect *fadeEffect;
    QPropertyAnimation     *fadeAnimation;

    QSize contentsSize() const;
    void  fitToContents();

  protected:
    void recalculateGeometry() override;
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event);
#endif
    void leaveEvent(QEvent *event) override;
};
