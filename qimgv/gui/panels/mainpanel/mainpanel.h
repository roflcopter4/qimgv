#pragma once

#include <QVBoxLayout>
#include "gui/customwidgets/slidepanel.h"
#include "gui/customwidgets/actionbutton.h"
#include "gui/panels/mainpanel/thumbnailstripproxy.h"

class MainPanel : public SlidePanel {
    Q_OBJECT
public:
    explicit MainPanel(FloatingWidgetContainer *parent);
    ~MainPanel() override;

    void  setPosition(PanelPosition) override;
    void  setExitButtonEnabled(bool mode);
    void  setupThumbnailStrip();
    QSize sizeHint() const override;
    auto  getThumbnailStrip() -> QSharedPointer<ThumbnailStripProxy>;

public slots:
    void readSettings();

signals:
    void pinned(bool mode);

private slots:
    void onPinClicked();

private:
    QVBoxLayout *buttonsLayout;
    QWidget *buttonsWidget;
    QSharedPointer<ThumbnailStripProxy> thumbnailStrip;
    ActionButton *openButton, *settingsButton, *exitButton, *folderViewButton, *pinButton;

protected:
    void paintEvent(QPaintEvent* event) override;
};
