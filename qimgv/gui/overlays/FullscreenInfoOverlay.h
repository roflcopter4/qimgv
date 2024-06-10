#pragma once

#include "gui/customWidgets/OverlayWidget.h"
#include <QWidget>

namespace Ui {
class FullscreenInfoOverlay;
}

class FullscreenInfoOverlay : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit FullscreenInfoOverlay(FloatingWidgetContainer *parent = nullptr);
    ~FullscreenInfoOverlay() override;
    void setInfo(QString const &pos, QString const &fileName, QString const &info);

  private:
    Ui::FullscreenInfoOverlay *ui;
};
