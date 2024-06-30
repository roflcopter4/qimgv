#pragma once

#include "gui/customWidgets/OverlayWidget.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

class ZoomIndicatorOverlay : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit ZoomIndicatorOverlay(FloatingWidgetContainer *parent = nullptr);

    void setScale(qreal scale);
    void show();
    void show(int duration);

  protected:
    void recalculateGeometry() override;

  private:
    std::unique_ptr<QFontMetrics> fm;
    QHBoxLayout *layout;
    QLabel       label;
    QTimer       visibilityTimer;
    int          hideDelay = 2000;
};
