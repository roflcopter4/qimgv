#pragma once

#include "gui/customWidgets/OverlayWidget.h"
#include "Settings.h"
#include <QPushButton>

namespace Ui {
class SaveConfirmOverlay;
}

class SaveConfirmOverlay : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit SaveConfirmOverlay(FloatingWidgetContainer *parent = nullptr);
    ~SaveConfirmOverlay() override;

  Q_SIGNALS:
    void saveClicked();
    void saveAsClicked();
    void discardClicked();

  private Q_SLOTS:
    void readSettings();

  private:
    Ui::SaveConfirmOverlay *ui;
};
