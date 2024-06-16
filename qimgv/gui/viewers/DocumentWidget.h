#pragma once

#include "gui/customWidgets/FloatingWidgetContainer.h"
#include "gui/panels/infoBar/InfoBarProxy.h"
#include "gui/panels/mainPanel/MainPanel.h"
#include "gui/viewers/ViewerWidget.h"
#include <QBoxLayout>
#include <memory>

// TODO: use a template here?

class DocumentWidget : public FloatingWidgetContainer
{
  public:
    DocumentWidget(ViewerWidget *viewWidget, InfoBarProxy *infoBar, QWidget *parent);
    ~DocumentWidget() override;

    ND ViewerWidget        *viewWidget();
    ND ThumbnailStripProxy *thumbPanel() const;

    ND bool panelEnabled() const;

    void setFocus();
    void hideFloatingPanel();
    void hideFloatingPanel(bool animated);
    void setPanelEnabled(bool mode);
    void setupMainPanel();
    void setInteractionEnabled(bool mode);
    void allowPanelInit();

  public Q_SLOTS:
    void onFullscreenModeChanged(bool mode);

  private Q_SLOTS:
    void setPanelPinned(bool mode);
    bool panelPinned() const;
    void readSettings();

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QBoxLayout   *layout;
    QBoxLayout   *layoutRoot;
    ViewerWidget *mViewWidget;
    InfoBarProxy *mInfoBar;
    MainPanel    *mainPanel;

    bool avoidPanelFlag       : 1 = false;
    bool mPanelEnabled        : 1 = false;
    bool mPanelFullscreenOnly : 1 = false;
    bool mIsFullscreen        : 1 = false;
    bool mPanelPinned         : 1 = false;
    bool mInteractionEnabled  : 1 = false;
    bool mAllowPanelInit      : 1 = false;
};
