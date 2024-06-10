#pragma once

#include "gui/customWidgets/ActionButton.h"
#include "gui/customWidgets/SlidePanel.h"
#include "gui/panels/mainPanel/ThumbnailStripProxy.h"
#include <QVBoxLayout>

class MainPanel : public SlidePanel {
    Q_OBJECT
public:
    explicit MainPanel(FloatingWidgetContainer *parent);
    ~MainPanel() override;

    void  setPosition(PanelPosition) override;
    void  setExitButtonEnabled(bool mode);
    void  setupThumbnailStrip();
    QSize sizeHint() const override;
    auto  getThumbnailStrip() -> ThumbnailStripProxy *;

public slots:
    void readSettings();

signals:
    void pinned(bool mode);

private slots:
    void onPinClicked();

private:
    QVBoxLayout *buttonsLayout;
    QWidget *buttonsWidget;
    ThumbnailStripProxy *thumbnailStrip;
    ActionButton *openButton, *settingsButton, *exitButton, *folderViewButton, *pinButton;

protected:
    void paintEvent(QPaintEvent* event) override;
};
